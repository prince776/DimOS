#pragma once

#include <kernel/memory/pmm.h>
#include <stdint.h>

using VirtualAddr = uint64_t;

// Paging::Table here refers to a generic table representing PT/PD/PDPT/PML4,
// not the PageTable that's pointed by PageDirectory.

/**
 * @brief Steps
 * Do HHDM for used up parts of Physcial Mempory
 * Do KH(kernel Half, last 2 GiB) mapping for kernel
 * Now VMM is gonna have modes, user/kernel
 * for kernel mode (only mode for now), VMM will start looking for free pages
 * starting from KH (2nd last PDE) For ease of impl, we can try to map 2GiB of
 * physical mem to KH, that puts a strict req on memory though, even though
 * kernel would probably need much less in the foreseeable future. This also
 * saves the trouble of keeping virtual address of PageTables, so that'd cut our
 * Paging Table cost by 50%
 */

namespace Paging {
enum FlagOffset {
    PRESENT_FLAG = 0,
    WRITABLE = 1,
    USER_ACCESSIBLE = 2,
    CACHING = 3,
    DISABLE_CACHE = 4,
    ACCESSED = 5,
    DIRTY = 6,
    HUGE_PAGE = 7,
    GLOBAL = 8,
    NO_EXECUTE = 63,
    TABLE_LEVEL =
        9,  // Custom flag to figure out what level this table belongs to
    TABLE_LEVEL_2 = 10,
};

constexpr uint64_t physicalAddrMask = 0x000ffffffffff000;
struct Entry {
    uint64_t data;

    Entry() : data(0) {}
    Entry(uint64_t value) : data(value) {}

    inline void setFlags(uint64_t flags) { data |= flags; }
    inline void removeFlags(uint64_t flags) { data &= (~flags); }
    inline bool testFlag(FlagOffset flagOffset) const {
        return ((data >> flagOffset) & 1);
    }

    inline PhysicalAddr getPhysicalAddr() const {
        return data & physicalAddrMask;
    }
    inline void setPhysicalAdr(PhysicalAddr addr) {
        removeFlags(physicalAddrMask);
        data |= (addr & physicalAddrMask);
    }

    inline bool isPresent() { return testFlag(PRESENT_FLAG); }
    inline bool isWriteable() { return testFlag(WRITABLE); }
    inline bool isUserAccessible() { return testFlag(USER_ACCESSIBLE); }
    inline bool hasCaching() { return testFlag(CACHING); }
    inline bool isCacheDisabled() { return testFlag(DISABLE_CACHE); }
    inline bool isAccessed() { return testFlag(ACCESSED); }
    inline bool isDirty() { return testFlag(DIRTY); }
    inline bool isGlobal() { return testFlag(GLOBAL); }
    inline bool isNoExecute() { return testFlag(NO_EXECUTE); }
};

constexpr int entryCnt = 512;
constexpr int defaultEntry = 0;

enum TableLevel : uint64_t {
    LEVEL0 = 0,
    LEVEL1 = 1,
    LEVEL2 = 2,
    LEVEL3 = 3,
};
// Table level can be determined by looking at TABLE_LEVEL bits (9, 10) of first
// entry
struct Table {
    Entry entries[entryCnt];
    Table* nextTables[entryCnt];
    Table() = default;

    void init(TableLevel tableLevel) {
        for (int i = 0; i < entryCnt; i++) {
            entries[i] = defaultEntry;
            nextTables[i] = nullptr;
        }
        setLevel(tableLevel);
    }

    inline TableLevel getLevel() const {
        return (
            TableLevel)(entries[0].testFlag(TABLE_LEVEL) +
                        ((TableLevel)entries[0].testFlag(TABLE_LEVEL_2) << 1));
    }

    inline void setLevel(TableLevel level) {
        entries[0].removeFlags((1LL << TABLE_LEVEL) | (1LL << TABLE_LEVEL_2));
        entries[0].setFlags(level << TABLE_LEVEL);
    }
};
};  // namespace Paging
