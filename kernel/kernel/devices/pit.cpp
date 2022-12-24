#include <kernel/devices/pit.h>

namespace pit {

    void init(uint16_t divisor) {
        // Temp code to init a timer
        pic::clearMask(IRQLine);

        // Send the command byte.
        io_outb(0x43, 0x36);

        // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
        uint8_t l = (uint8_t)(divisor & 0xFF);
        uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

        // Send the frequency divisor.
        io_outb(0x40, l);
        io_outb(0x40, h);
    }

    void stop() {
        pic::maskLine(IRQLine);
    }

}