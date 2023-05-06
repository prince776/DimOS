#include <kernel/debug.h>

extern struct limine_terminal_request terminal_request;

extern "C" void limineDebug(const char* str, int len) {
    struct limine_terminal* terminal = terminal_request.response->terminals[0];
    terminal_request.response->write(terminal, str, len);
}
