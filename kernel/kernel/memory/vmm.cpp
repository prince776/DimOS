#include <kernel/memory/vmm.h>
#include <kernel/memory/paging.h>
#include <kernel/common.h>
#include <kernel/isr.h>

extern uint64_t HHDMOffset;
extern "C" void loadPageDirectory(uint64_t);

void VMM::init(const PhysicalMemMap& physicalMemMap, const MemRange* otherRanges, uint64_t otherRangesLen) {
    auto& pmm = PMM::get();
    physicalMemUsage = { 0, 0 };

    pml4 = (Paging::Table*)(pmm.allocNFrames(2) + HHDMOffset);
    physicalMemUsage.count += 2;
    physicalMemUsage.lastAddr = (PhysicalAddr)(pml4 - HHDMOffset + PMM::frameSize);
    pml4->init(Paging::LEVEL0);

    // Map kernel in KH (Kernel Half, from 0xffffffff80000000)
    {
        IndexOffset kernelIdxOffset(getIndexOffset(0xffffffff80000000));
        auto& i = kernelIdxOffset.i;
        printf("kernel offset values: %d, %d, %d, %d\n", i[0], i[1], i[2], i[3]);
        for (auto st = physicalMemMap.kernelStart; st < physicalMemMap.kernelStart + physicalMemMap.kernelSize; st += pageSize) {
            kernelIdxOffset = getFirstUnallocPage(kernelIdxOffset, USE_PMM);
            auto* pte = getPTE(kernelIdxOffset);
            assignFrameToPage(pte, st, true);
        }
        availablePagesHH = 0;
        availablePagesLH = 0;
    }
    // Map the used parts of physical memory with HHDM
    // Also do the identity mapping, can be removed once framebufer driver is written
    {
        auto setHHDMandIdentityPTE = [&](const MemRange& curr) {
            if (curr.size >= pageSize) {
                printf("[VMM] Mapping Physical range: %x, %x\n", curr.start, curr.start + curr.size);
                for (auto currSt = curr.start; currSt < curr.start + curr.size; currSt += pageSize) {
                    auto identityIndexOffset = getFirstUnallocPage(getIndexOffset(currSt), USE_PMM);
                    auto hhdmIndexOffset = getFirstUnallocPage(getIndexOffset(currSt + HHDMOffset), USE_PMM);

                    auto* pte = getPTE(hhdmIndexOffset);
                    assignFrameToPage(pte, currSt, true);

                    pte = getPTE(identityIndexOffset);
                    assignFrameToPage(pte, currSt, false);
                }
            }
        };
        uint64_t st = 0;
        for (int i = 0; i < physicalMemMap.availMemArrCnt; i++) {
            auto& memEntry = physicalMemMap.availableMemArr[i];
            MemRange curr = { st, memEntry.start - st };
            setHHDMandIdentityPTE(curr);
            st = memEntry.start + memEntry.size;
        }
        MemRange curr = { st, physicalMemMap.totalSize - st };
        setHHDMandIdentityPTE(curr);

        // Map the other ranges as well
        for (int i = 0; i < otherRangesLen; i++) {
            setHHDMandIdentityPTE(otherRanges[i]);
        }

        MemRange physicalMemUsageRange = {
            physicalMemUsage.lastAddr - physicalMemUsage.count * PMM::frameSize,
            physicalMemUsage.count * PMM::frameSize
        };
        setHHDMandIdentityPTE(physicalMemUsageRange);

        // Allocate some free pages, just in case
        physicalMemUsageRange.start += physicalMemUsageRange.size;
        for (auto currSt = physicalMemUsageRange.start; currSt < physicalMemUsageRange.start + physicalMemUsageRange.size; currSt += pageSize) {
            auto identityIndexOffset = getFirstUnallocPage(getIndexOffset(currSt), USE_PMM);
            auto hhdmIndexOffset = getFirstUnallocPage(getIndexOffset(currSt + HHDMOffset), USE_PMM);
        }
    }

    printf("[VMM] Initialization finished with available pages-> HH: %l, LH: %l\n", availablePagesHH, availablePagesLH);
    switchPML4(pml4);
    registerInterruptHandler(14, pageFaultHanlder);
}

bool VMM::switchPML4(Paging::Table* pml4) {
    if (!pml4)
        return false;
    this->pml4 = pml4;
    loadPageDirectory((uint64_t)this->pml4 - HHDMOffset);
    return true;
}

void VMM::assignFrameToPage(Paging::Entry* pte, PhysicalAddr frameAddr, bool higherHalf) {
    pte->setPhysicalAdr(frameAddr);
    pte->setFlags((1LL << Paging::PRESENT_FLAG) | (1LL << Paging::WRITABLE));
    if (higherHalf) availablePagesHH--;
    else availablePagesLH--;
}

VirtualAddr VMM::allocPage(bool higherHalf) {
    return allocPages(higherHalf, true, 1);
}

VirtualAddr VMM::allocNPages(bool higherHalf, int n) {
    return allocPages(higherHalf, true, n);
}

void VMM::freePage(VirtualAddr addr) {
    IndexOffset idxOffset = getIndexOffset(addr);
    auto* pte = getPTE(idxOffset);

    PMM::get().freeFrame(pte->getPhysicalAddr());
    *pte = Paging::defaultEntry;

    bool higherHalf = idxOffset.getVirtualAddr() >= HHDMOffset;
    if (higherHalf) availablePagesHH++;
    else availablePagesLH++;
    invlpg(addr);
}

IndexOffset VMM::getFirstUnallocPage(IndexOffset begin, VMM::AllocScheme allocScheme) {
    bool done = getFirstUnallocPageUtil(begin, allocScheme, pml4);
    if (!done) {
        begin.i[0] = 0;
        begin.i[1] = 0;
        begin.i[2] = 0;
        begin.i[3] = 0;
    }
    return begin;
}

IndexOffset VMM::getFirstNUnallocPage(IndexOffset begin, VMM::AllocScheme allocScheme, int n) {
    IndexOffset res = getFirstUnallocPage(begin, allocScheme);
    auto prev = res;
    for (int done = 1; done < n;) {
        auto next = getFirstUnallocPage(prev, allocScheme);
        if (next.getVirtualAddr() - res.getVirtualAddr() == pageSize) {
            done++;
            prev = next;
        }
        else { // didn't get a continuous area, restart
            res = next;
            prev = next;
            done = 1;
        }
    }
    return res;
}

bool VMM::getFirstUnallocPageUtil(IndexOffset& curr, VMM::AllocScheme allocScheme, Paging::Table* table) {
    auto prev = curr;
    bool done = false;
    auto level = table->getLevel();
    for (int i = curr.i[level]; i < entriesPerTable && !done; i++) {

        if (i > curr.i[level]) {
            for (int j = level + 1; j < VMMTableLevelCnt; j++) {
                curr.i[j] = 0;
            }
        }

        if (level == Paging::LEVEL3) {
            if (!table->entries[i].isPresent()) {
                curr.i[level] = i;
                done = true;
            }
        }
        else {
            if (!table->entries[i].isPresent()) {
                if (allocScheme == NO_ALLOC) {
                    continue;
                }
                Paging::Table* nextTable;
                PhysicalAddr paddr;
                if (allocScheme == USE_PMM) {
                    // TODO: Don't really need to allocate 2 frames for the last level, since nextTable array won't ever be used by it.
                    paddr = PMM::get().allocNFrames(2);
                    physicalMemUsage.count += 2;
                    physicalMemUsage.lastAddr = paddr + PMM::frameSize;

                    nextTable = (Paging::Table*)(paddr + HHDMOffset); // This calc only really works while Limine pages are loaded, hence using PMM
                    // Hence this depends on the fact that the memory being used here will be covered by the paging tables manually
                }
                else if (allocScheme == USE_VMM) {
                    // This depends on guarantee that there are enough avilable pages, which should work as long as there are enough after VMM::init()
                    paddr = PMM::get().allocNFrames(2);
                    // search for pages to use for VMM memory in HH only
                    IndexOffset firstPageOffset = getFirstNUnallocPage(IndexOffset(256, 0, 0, 0), NO_ALLOC, 2);
                    nextTable = (Paging::Table*)firstPageOffset.getVirtualAddr();
                }
                nextTable->init((Paging::TableLevel)(level + 1));
                table->entries[i].setFlags((1LL << Paging::PRESENT_FLAG) | (1LL << Paging::WRITABLE));
                table->entries[i].setPhysicalAdr(paddr);
                table->nextTables[i] = nextTable;

                if (nextTable->getLevel() == Paging::LEVEL3) { // we create a new Table of LEVEL3
                    bool higherHalf = prev.getVirtualAddr() >= HHDMOffset;
                    if (higherHalf) availablePagesHH += entriesPerTable;
                    else availablePagesLH += entriesPerTable;
                }

            }
            done = getFirstUnallocPageUtil(curr, allocScheme, table->nextTables[i]);
            if (done) {
                curr.i[level] = i;
            }
        }
    }
    return done;
}

VirtualAddr VMM::allocPages(bool higherHalf, bool doMinCheck, int reqPages) {
    int availablePages = getAvailablePages(higherHalf);
    int minReqPages = minAvailablePages + reqPages;
    IndexOffset beginOffset(0, 0, 0, 0);
    if (higherHalf) {
        beginOffset.i[0] = 256;
    }

    if (doMinCheck && availablePages < minReqPages) {
        getFirstNUnallocPage(beginOffset, USE_VMM, minReqPages);
    }
    auto& pmm = PMM::get();
    if (pmm.frameCnt - pmm.usedframes < reqPages) {
        panic("[VMM] OUT OF REQUESTABLE PHYSICAL MEMORY\n");
    }

    IndexOffset firstPageOffset = getFirstNUnallocPage(beginOffset, NO_ALLOC, reqPages);
    VirtualAddr firstPage = firstPageOffset.getVirtualAddr();
    auto currOffset = firstPageOffset;
    for (int i = 0; i < reqPages; i++) {
        PhysicalAddr frameAddr = pmm.allocFrame();
        assignFrameToPage(getPTE(currOffset), frameAddr, currOffset.getVirtualAddr() >= HHDMOffset);
        currOffset = currOffset.nextPage();
    }
    return firstPage;
}