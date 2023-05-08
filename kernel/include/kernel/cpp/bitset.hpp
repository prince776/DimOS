#pragma once
#include <stdint.h>

#include <kernel/cpp/vector.hpp>

template <Allocator Alloc = Mallocator> class Bitset {
  public:
    static constexpr int BlockSize = 64;
    static constexpr uint64_t BlockMask = ~(0ULL);

    class BitProxy {
        Bitset& ref;
        int idx{}, offset{};

      public:
        BitProxy(Bitset& ref, int idx, int offset) : ref(ref), idx(idx), offset(offset) {}
        operator bool() const noexcept { return (ref.data[idx] & (1LL << offset)); }
        void operator=(uint8_t val) noexcept {
            ref.data[idx] |= (1LL << offset);
            if (!val) {
                ref.data[idx] ^= (1LL << offset);
            }
        }
    };

    Bitset() = default;
    Bitset(size_t size) {
        Alloc allocator{};
        len = (size + BlockSize - 1) / BlockSize;
        data = Vector<uint64_t, Alloc>(len, 0, allocator);
    }
    Bitset(size_t size, Alloc allocator) {
        len = (size + BlockSize - 1) / BlockSize;
        data = Vector<uint64_t, Alloc>(len, allocator);
    }

    BitProxy operator[](size_t idx) noexcept {
        return BitProxy(*this, idx / BlockSize, idx % BlockSize);
    }

    void flip() noexcept {
        for (auto& v : data) {
            v ^= BlockMask;
        }
    }

  private:
    Vector<uint64_t, Alloc> data;
    int len;
};
