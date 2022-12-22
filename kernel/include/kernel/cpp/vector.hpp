#pragma once

#include <kernel/cpp/iterator.hpp>
#include <kernel/cpp/unique-ptr.hpp>

template <typename T>
class Vector {
public:
    // Constructors
    Vector() = default;
    Vector(size_t size): size(size), capacity(size) {
        data = makeUnique<T[]>(capacity);
        fill(T{});
    }
    Vector(size_t size, const T& val): size(size), capacity(size) {
        data = makeUnique<T[]>(capacity);
        fill(val);
    }
    // Vector(std::initializer_list<T> ini)
    //     : size(ini.size()), capacity(ini.size()) {
    //     data = makeUnique<T[]>(capacity);
    //     auto it = begin();
    //     for (const auto& val : ini) {
    //         *it = val;
    //         it++;
    //     }
    // }

    void resize(size_t newSize) {
        auto newCapacity = capacity;
        if (newSize > capacity) {
            newCapacity = capacity * 2;
        }
        realloc(newSize, newCapacity);
    }

    void push_back(const T& val) {
        if (capacity == size) {
            realloc(size, capacity * 2);
        }
        data[size++] = val;
    }

    void pop_back() noexcept {
        assert(size > 0);
        size--;
    }

    T& front() { return data[0]; }
    T& back() { return data[size - 1]; }

    // Iterators
    ForwardIterator<T> begin() noexcept { return ForwardIterator(&data[0]); }
    ForwardIterator<T> end() noexcept {
        return ForwardIterator(&data[0] + size);
    }

    // Random access
    T& operator[](size_t idx) noexcept { return data[idx]; }
    const T& operator[](size_t idx) const noexcept { return data[idx]; }
    T& at(size_t idx) const noexcept {
        assert(idx >= size);
        return (*this)[idx];
    }

private:
    void fill(const T& val) {
        for (size_t i = 0; i < size; i++) {
            data[i] = val;
        }
    }

    void realloc(size_t newSize, size_t newCapacity) {
        assert(newSize <= newCapacity);

        UniquePtr<T[]> newData = makeUnique<T[]>(newCapacity);
        size_t minSize = min(size, newSize);
        for (size_t i = 0; i < minSize; i++) {
            newData[i] = data[i];
        }
        for (size_t i = minSize; i < size; i++) {
            newData[i] = T{};
        }
        auto newDataPtr = newData.release();
        data.reset(newDataPtr);
        size = newSize;
        capacity = newCapacity;
    }

private:
    UniquePtr<T[]> data;
    size_t size = 0, capacity = 0;
};
