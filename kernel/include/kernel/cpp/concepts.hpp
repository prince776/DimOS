#pragma once
#include <kernel/cpp/type-traits.hpp>

template <typename T, typename U>
concept same_as = is_same_v<T, U>;
