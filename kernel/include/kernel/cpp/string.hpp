#pragma once

#include <kernel/cpp/iterator.hpp>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/vector.hpp>
#include <string.h>

template <Allocator Alloc = Mallocator>
class String: public Vector<char, Alloc> {
public:
    String() = default;
    String(size_t size, Alloc allocator = {}) {
        this->allocator = allocator;
        this->m_size = size + 1;
        this->capacity = size + 1;
        this->data = makeUnique<char[]>(this->allocator, this->capacity);
        this->fill(char{});
        this->data[this->m_size - 1] = 0;
    }
    String(size_t size, char val, Alloc allocator = {}) {
        this->allocator = allocator;
        this->m_size = size + 1;
        this->capacity = size + 1;
        this->data = makeUnique<char[]>(this->allocator, this->capacity);
        this->fill(val);
        this->data[this->m_size - 1] = 0;
    }
    String(const char* str, Alloc allocator = {}) {
        this->allocator = allocator;
        size_t len = strlen(str);
        this->capacity = len + 1;
        this->m_size = len + 1;
        this->data = makeUnique<char[]>(this->allocator, this->capacity);
        for (int i = 0; i < len; i++) {
            this->data[i] = str[i];
        }
        this->data[len] = 0;
    }
    String(const String<Alloc>& s) {
        this->m_size = s.m_size;
        this->capacity = s.capacity;
        this->allocator = s.allocator;
        this->data.~UniquePtr();
        this->data = makeUnique<char[]>(this->allocator, this->capacity);
        for (int i = 0; i < this->m_size; i++) {
            (*this)[i] = s[i];
        }
    }
    String& operator=(const String<Alloc>& s) {
        this->m_size = s.m_size;
        this->capacity = s.capacity;
        this->allocator = s.allocator;
        this->data.~UniquePtr();
        this->data = makeUnique<char[]>(this->allocator, this->capacity);
        for (int i = 0; i < this->m_size; i++) {
            (*this)[i] = s[i];
        }
        return *this;
    }

    size_t size() const { return this->m_size - 1; }

    String& operator+=(const String& rhs) {
        this->pop_back();
        for (const auto& val : rhs) {
            push_back(val);
        }
    }

    String operator+(const String& rhs) const {
        String res = *this;
        res.pop_back();
        for (const auto& val : rhs) {
            res.push_back(val);
        }
        return res;
    }

    String substr(int offset, size_t size) const {
        String res(size);
        for (int i = 0; i < size; i++) {
            res[i] = (*this)[i + offset];
        }
        return res;
    }

    int npos = -1;
    int find(char c, int begin = 0) const {
        for (int i = begin; i < size(); i++) {
            if ((*this)[i] == c) {
                return i;
            }
        }
        return npos;
    }

    Vector<String> split(char delimiter) const {
        Vector<String> res;
        int l = 0;
        int r = find(delimiter, l);
        while (r != npos) {
            res.push_back(substr(l, r - l));
            l = r + 1;
            r = find(delimiter, l);
        }
        res.push_back(substr(l, this->m_size - l));
        return res;
    }

    const char* c_str() const { return this->data.get(); }
};
