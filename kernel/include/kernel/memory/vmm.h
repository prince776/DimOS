#pragma once
#include <stdio.h>
#include <stdint-gcc.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/paging.h>

// IMP THING:
// VirtualAddr of PTE != VirtualAddr of Page it helps translate to
// it's obvious in hindsight, but easy to miss.

constexpr int VMMTableLevelCnt = 4;
struct IndexOffset {
    uint64_t i[VMMTableLevelCnt] = { 0 };
    IndexOffset() = default;

    IndexOffset(uint64_t i0, uint64_t i1, uint64_t i2, uint64_t i3) {
        i[0] = i0;
        i[1] = i1;
        i[2] = i2;
        i[3] = i3;
    }

    // MUST: whenever comparing with HHDM to check if vAddr is in higher half, convert to IndexOffset first.
    inline VirtualAddr getVirtualAddr() const {
        uint64_t ans = ((i[0] << 39) | (i[1] << 30) | (i[2] << 21) | (i[3] << 12));
        if (ans >= 0x800000000000) {
            ans |= 0xffff000000000000;
        }
        return ans;
    }

    IndexOffset nextPage() const {
        IndexOffset res = (*this);
        int carry = 1;
        for (int i = 3; i >= 0; i--)
        {
            res.i[i] += carry;
            carry = 0;
            if (res.i[i] >= Paging::entryCnt) {
                carry = 1;
                res.i[i] = 0;
            }
        }
        return res;
    }
};

struct PageInfo {
    Paging::Entry* pte;
    IndexOffset idx;

    PageInfo() : pte(nullptr), idx() {}

    inline VirtualAddr getVirtualAddr() const {
        return idx.getVirtualAddr();
    }
};

class VMM {
public:
    static constexpr int64_t entriesPerTable = Paging::entryCnt;
    static constexpr int64_t pageSize = 4096;
    static constexpr int64_t pageAlignment = 4096;

private:
    Paging::Table* pml4 = nullptr;
    int64_t availablePagesLH = 0; // with every new table(level:3) added, availablePages += entriesPerTable, with every page allocated, availablePages--
    int64_t availablePagesHH = 0; // For Higher half

    enum AllocScheme {
        NO_ALLOC = 0,
        USE_PMM,
        USE_VMM,
    };
private:
    VMM() {}
    VMM(const VMM&);
    void operator=(const VMM&);
public:
    static VMM& get() {
        static VMM vmm;
        return vmm;
    }
    /**
     * @brief Steps:
     * Do HHDM(Higher Half Direct Mapping 0xffff8...) for used up parts of Physcial Memory
     * Do KH(kernel Half) mapping for kernel
     * Now VMM is gonna have modes, user/kernel
     * for kernel mode (only mode for now), VMM will start looking for free pages starting from KH (2nd last PDE)
     * For ease of impl, we can try to map 2GiB of physical mem to KH, that puts a strict req on memory though,
     * even though kernel would probably need much less in the foreseeable future. This also saves the trouble of
     * keeping virtual address of PageTables, so that'd cut our Paging Table cost by 50%
     */
    void init(const PhysicalMemMap& physicalMemMap, const MemRange* otherRanges, uint64_t otherRangesLen);

    inline int getI3(VirtualAddr x) const { return (((x) >> 12) & 0x1ff); }
    inline int getI2(VirtualAddr x) const { return (((x) >> 21) & 0x1ff); }
    inline int getI1(VirtualAddr x) const { return (((x) >> 30) & 0x1ff); }
    inline int getI0(VirtualAddr x) const { return (((x) >> 39) & 0x1ff); }
    inline int64_t getAvailablePages(bool higherHalf) {
        return higherHalf ? availablePagesHH : availablePagesLH;
    }

    inline IndexOffset getIndexOffset(VirtualAddr x) const {
        IndexOffset res(getI0(x), getI1(x), getI2(x), getI3(x));
        return res;
    }

    Paging::Entry* getPTE(const IndexOffset& offsets) {
        auto& i = offsets.i;
        return &pml4->nextTables[i[0]]->nextTables[i[1]]->nextTables[i[2]]->entries[i[3]];
    }


    bool switchPML4(Paging::Table* pml4);
    inline Paging::Table* getPML4() { return pml4; }

    void freePage(VirtualAddr addr);
    VirtualAddr allocPage(bool higherHalf);
    VirtualAddr allocNPages(bool higherHalf, int n);
    void assignFrameToPage(Paging::Entry* pte, PhysicalAddr frameAddr, bool higherHalf);

    inline void invlpg(VirtualAddr addr) {
        // asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
        asm volatile("invlpg (%0)" : : "r" (addr));
    }

private:
    IndexOffset getFirstUnallocPage(IndexOffset begin, AllocScheme allocScheme);
    IndexOffset getFirstNUnallocPage(IndexOffset begin, AllocScheme allocScheme, int n);
    bool getFirstUnallocPageUtil(IndexOffset& curr, AllocScheme allocScheme, Paging::Table* table);
    VirtualAddr allocPages(bool higherHalf, bool doMinCheck, int reqPages);
    static constexpr int minAvailablePages = 11;

    struct PhysicalMemUsage {
        uint64_t count;
        PhysicalAddr lastAddr;
    } physicalMemUsage;
};
