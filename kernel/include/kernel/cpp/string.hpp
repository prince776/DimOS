#pragma once

#include <kernel/cpp/iterator.hpp>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/vector.hpp>
#include <string.h>

class String: public Vector<char> {
public:
    String() = default;
    String(size_t size) {
        m_size = size + 1;
        capacity = size + 1;
        data = makeUnique<char[]>(capacity);
        fill(char{});
        data[m_size - 1] = 0;
    }
    String(size_t size, char val) {
        m_size = size + 1;
        capacity = size + 1;
        data = makeUnique<char[]>(capacity);
        fill(val);
        data[m_size - 1] = 0;
    }
    String(const char str[]) {
        size_t len = strlen(str);
        capacity = len + 1;
        m_size = len + 1;
        data = makeUnique<char[]>(capacity);
        for (int i = 0; i < len; i++) {
            data[i] = str[i];
        }
        data[len] = 0;
    }

    String(const String& s) {
        m_size = s.m_size;
        capacity = s.capacity;
        data = makeUnique<char[]>(capacity);
        for (int i = 0; i < m_size; i++) {
            (*this)[i] = s[i];
        }
    }
    String& operator=(const String& s) {
        m_size = s.m_size;
        capacity = s.capacity;
        data = makeUnique<char[]>(capacity);
        for (int i = 0; i < m_size; i++) {
            (*this)[i] = s[i];
        }
        return *this;
    }

    size_t size() const { return m_size - 1; }


    String& operator+=(const String& rhs) {
        pop_back();
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
        res.push_back(substr(l, m_size - l));
        return res;
    }

    const char* c_str() const { return data.get(); }
};
