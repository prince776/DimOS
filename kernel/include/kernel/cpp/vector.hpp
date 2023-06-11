#pragma once

#include <kernel/cpp/iterator.hpp>
#include <kernel/cpp/unique-ptr.hpp>

template <typename T, Allocator Alloc = Mallocator> class Vector {
  public:
    // Constructors
    Vector(Alloc allocator = {}) : allocator(allocator) {}
    Vector(size_t size, Alloc allocator = {}) : m_size(size), capacity(size), allocator(allocator) {
        data = makeUnique<T[], Alloc>(allocator, capacity);
        fill(T{});
    }
    Vector(size_t size, const T& val, Alloc allocator = {}) : m_size(size), capacity(size), allocator(allocator) {
        data = makeUnique<T[], Alloc>(allocator, capacity);
        fill(val);
    }
    Vector(const Vector<T, Alloc>& v) {
        allocator = v.allocator;
        m_size = v.size();
        capacity = v.capacity;
        data.~UniquePtr();
        data = makeUnique<T[], Alloc>(allocator, capacity);
        for (int i = 0; i < v.size(); i++) {
            data[i] = v[i];
        }
    }
    Vector& operator=(const Vector<T, Alloc>& v) {
        allocator = v.allocator;
        m_size = v.size();
        capacity = v.capacity;
        data.~UniquePtr();
        data = makeUnique<T[], Alloc>(allocator, capacity);
        for (int i = 0; i < v.size(); i++) {
            data[i] = v[i];
        }
        return *this;
    }
    // Vector(std::initializer_list<T> ini, Alloc allocator = {})
    //     : m_size(ini.size()), capacity(ini.size()), allocator(allocator) {
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
            newCapacity = capacity * 2 + 1;
        }
        realloc(newSize, newCapacity);
    }

    void push_back(const T& val) {
        if (capacity == m_size) {
            realloc(m_size, m_size * 2 + 1);
        }
        data[m_size++] = val;
    }

    void push_back(T&& val) {
        if (capacity == m_size) {
            realloc(m_size, m_size * 2 + 1);
        }
        data[m_size++] = move(val);
    }

    void pop_back() noexcept {
        assert(m_size > 0);
        m_size--;
    }

    size_t size() const { return m_size; }

    T& front() { return data[0]; }
    T& back() { return data[m_size - 1]; }

    // Iterators
    ForwardIterator<T> begin() noexcept { return ForwardIterator(&data[0]); }
    ForwardIterator<T> end() noexcept { return ForwardIterator(&data[0] + m_size); }
    const ForwardIterator<const T> begin() const noexcept { return ForwardIterator(&data[0]); }
    const ForwardIterator<const T> end() const noexcept { return ForwardIterator(&data[0] + m_size); }

    // Random access
    T& operator[](size_t idx) noexcept { return data[idx]; }
    const T& operator[](size_t idx) const noexcept { return data[idx]; }
    T& at(size_t idx) const noexcept {
        assert(idx < m_size);
        return (*this)[idx];
    }

    bool operator==(const Vector& other) const noexcept {
        if (size() != other.size()) {
            return false;
        }
        for (int i = 0; i < size(); i++) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }
        return true;
    }
    bool operator!=(const Vector& other) const noexcept { return !(*this == other); }

    int find(T c, int begin = 0) const {
        for (int i = begin; i < size(); i++) {
            if ((*this)[i] == c) {
                return i;
            }
        }
        return npos;
    }

    // swaps the element to back and calls pop_back().
    // Side effect: changes ordering of elements.
    void fastErase(int pos) {
        if (pos == npos)
            return;
        if (pos != (size() - 1)) {
            swap(this->operator[](pos), this->operator[](size() - 1));
        }
        pop_back();
    }

    static const int npos = -1;
    void reverse() {
        for (int i = 0; i < size() / 2; i++) {
            swap(data[i], data[size() - 1 - i]);
        }
    }
    void fill(const T& val) {
        for (size_t i = 0; i < m_size; i++) {
            data[i] = val;
        }
    }

  protected:
    void realloc(size_t newSize, size_t newCapacity) {
        assert(newSize <= newCapacity);

        UniquePtr<T[]> newData = makeUnique<T[]>(newCapacity);
        size_t minSize = min(m_size, newSize);
        for (size_t i = 0; i < minSize; i++) {
            newData[i] = move(data[i]);
        }
        for (size_t i = minSize; i < newSize; i++) {
            newData[i] = T{};
        }
        auto newDataPtr = newData.release();
        data.reset(newDataPtr);
        m_size = newSize;
        capacity = newCapacity;
    }

  protected:
    UniquePtr<T[], Alloc> data;
    Alloc allocator;
    size_t m_size = 0, capacity = 0;
};
