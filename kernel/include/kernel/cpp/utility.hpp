#pragma once
#include <kernel/common.h>
#include <kernel/cpp/type-traits.hpp>
#include <stddef.h>

template <typename T> [[nodiscard]] constexpr T&& forward(remove_reference_t<T>& t) {
    return static_cast<T&&>(t);
}

template <typename T> [[nodiscard]] constexpr remove_reference_t<T>&& move(T&& t) {
    return static_cast<remove_reference_t<T>&&>(t);
}

constexpr void assert(bool condition) {
    if (!condition) {
        panic("Assert failed.");
    }
}
constexpr void assert(bool condition, const char* msg) {
    if (!condition) {
        panic(msg);
    }
}

template <typename T> constexpr const T& min(const T& a, const T& b) {
    if (a < b) {
        return a;
    }
    return b;
}

template <typename T> constexpr const T& max(const T& a, const T& b) {
    if (a > b) {
        return a;
    }
    return b;
}

template <typename T> void swap(T& a, T& b) {
    T temp = move(a);
    a = move(b);
    b = move(temp);
}

///////// Placement new/delete //////////
inline void* operator new(size_t, void* p) throw() { return p; }
inline void* operator new[](size_t, void* p) throw() { return p; }
inline void operator delete(void*, void*) throw(){};
inline void operator delete[](void*, void*) throw(){};
