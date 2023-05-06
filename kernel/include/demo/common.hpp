
#pragma once
#include <kernel/cpp/string.hpp>

struct Command {
    String<> bin;
    Vector<String<>> args;
};
