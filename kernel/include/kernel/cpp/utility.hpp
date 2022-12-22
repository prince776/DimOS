#pragma once
#include <kernel/cpp/type-traits.hpp>
#include <kernel/common.h>

template<typename T>
[[ nodiscard ]]
constexpr T&& forward(remove_reference_t<T>& t)
{
    return static_cast<T&&>(t);
}

template<typename T>
[[ nodiscard ]]
constexpr remove_reference_t<T>&& move(T&& t)
{
    return static_cast<remove_reference_t<T>&&>(t);
}

constexpr void assert(bool condition) {
    if (!condition) {
        panic("Assert failed.");
    }
}
