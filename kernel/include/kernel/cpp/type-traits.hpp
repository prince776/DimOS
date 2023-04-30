#pragma once
#include <stddef.h>

/// is_array
template<typename>
struct is_array {
    static constexpr bool value = false;
};

template<typename T, size_t size>
struct is_array<T[size]> {
    static constexpr bool value = true;
};

template<typename T>
struct is_array<T[]> {
    static constexpr bool value = true;
};

template <typename T>
inline constexpr bool is_array_v = is_array<T>::value;


/// remove_extent
template<typename T>
struct remove_extent {
    using type = T;
};

template<typename T, size_t size>
struct remove_extent<T[size]> {
    using type = T;
};

template<typename T>
struct remove_extent<T[]> {
    using type = T;
};

template <typename T>
using remove_extent_t = typename remove_extent<T>::type;

/// remove_reference
template<typename T>
struct remove_reference {
    using type = T;
};

template<typename T>
struct remove_reference<T&> {
    using type = T;
};

template<typename T>
struct remove_reference<T&&> {
    using type = T;
};

template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

/// is_same
template<typename T, typename U>
struct is_same {
    static constexpr bool value = false;
};

template<typename T>
struct is_same<T, T> {
    static constexpr bool value = true;
};

template <typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;
