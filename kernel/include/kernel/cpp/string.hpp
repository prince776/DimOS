#pragma once
#include <string.h>

#include <kernel/cpp/allocator.hpp>
#include <kernel/cpp/iterator.hpp>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/vector.hpp>

template <Allocator Alloc = Mallocator> class String {
  public:
    String() = default;
    String(size_t size, Alloc allocator = {}) {
        data = Vector<char, Alloc>(size + 1, allocator);
        data.back() = '\0';
    }

    String(size_t size, char val, Alloc allocator = {}) {
        data = Vector<char, Alloc>(size + 1, val, allocator);
        data.back() = '\0';
    }

    String(const char* str, Alloc allocator = {}) {
        size_t len = strlen(str);
        data = Vector<char, Alloc>(len + 1, allocator);
        for (int i = 0; i < len; i++) {
            data[i] = str[i];
        }
        data.back() = '\0';
    }

    size_t size() const { return data.size() - 1; }

    String& operator+=(const String& rhs) {
        data.pop_back();
        for (const auto& val : rhs) {
            data.push_back(val);
        }
        data.push_back('\0');
        return *this;
    }

    String& operator+=(char c) {
        push_back(c);
        return *this;
    }

    String operator+(const String& rhs) const {
        String res = *this;
        res += rhs;
        return res;
    }

    void push_back(const char val) {
        data.back() = val;
        data.push_back('\0');
    }

    void pop_back() noexcept {
        data.pop_back();
        data.back() = '\0';
    }

    char& front() { return data[0]; }
    char& back() { return data[size() - 1]; }

    // Iterators
    ForwardIterator<char> begin() noexcept { return ForwardIterator(&data[0]); }
    ForwardIterator<char> end() noexcept { return ForwardIterator(&data[0] + size()); }
    const ForwardIterator<const char> begin() const noexcept { return ForwardIterator(&data[0]); }
    const ForwardIterator<const char> end() const noexcept {
        return ForwardIterator(&data[0] + size());
    }

    // Random access
    char& operator[](size_t idx) noexcept { return data[idx]; }
    const char& operator[](size_t idx) const noexcept { return data[idx]; }
    char& at(size_t idx) const noexcept {
        assert(idx < size());
        return (*this)[idx];
    }

    bool operator==(const String& other) const noexcept { return data == other.data; }

    bool operator!=(const String& other) const noexcept { return !(*this == other); }

    String substr(int offset, size_t size) const {
        String res(size);
        for (int i = 0; i < size; i++) {
            res[i] = (*this)[i + offset];
        }
        return res;
    }

    int find(const char val, int begin) const {
        auto pos = data.find(val, begin);
        if (pos >= size() || pos == Vector<char, Alloc>::npos) {
            pos = npos;
        }
        return pos;
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
        if (size() == 0 || l != size()) { // todo empty str case
            res.push_back(substr(l, size() - l));
        }
        return res;
    }

    const char* c_str() const { return &data[0]; }

    void reverse() {
        for (int i = 0; i < size() / 2; i++) {
            swap(data[i], data[size() - 1 - i]);
        }
    }
    static const int npos = -1;

  private:
    Vector<char, Alloc> data;
};

inline String<> stoi(int n) {
    String<> res = "";
    while (n > 0) {
        res += ('0' + (n % 10));
        n /= 10;
    }
    res.reverse();
    return res;
}
