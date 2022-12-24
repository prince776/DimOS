#include <kernel/devices/pic.h>
#include <kernel/x64.h>

namespace pic {
    void disable() {
        io_outb(PIC2_DATA, 0xFF);
        io_outb(PIC1_DATA, 0xFF);
    }

    void remap() {
        uint8_t mask1 = io_inb(PIC1_DATA);
        uint8_t mask2 = io_inb(PIC2_DATA);


        io_outb(PIC1_COMMAND, 0x11);
        io_outb(PIC2_COMMAND, 0x11);
        io_wait();


        io_outb(PIC1_DATA, PIC1Offset); // pic1 offset in IDT
        io_outb(PIC2_DATA, PIC2Offset); // pic2 offset in IDT
        io_wait();

        io_outb(PIC1_DATA, 0x04);
        io_outb(PIC2_DATA, 0x02);
        io_wait();

        io_outb(PIC1_DATA, 0x01);
        io_outb(PIC2_DATA, 0x01);
        io_wait();

        io_outb(PIC1_DATA, 0x00);
        io_outb(PIC2_DATA, 0x00);
        io_wait();


        io_outb(PIC1_DATA, ICW4_8086);
        io_wait();
        io_outb(PIC2_DATA, ICW4_8086);
        io_wait();


        io_outb(PIC1_DATA, mask1);
        io_outb(PIC2_DATA, mask2);
    }

    void maskLine(uint8_t irqLine) {
        uint16_t port;
        uint8_t value;

        if (irqLine < 8)
            port = PIC1_DATA;

        else
        {
            port = PIC2_DATA;
            irqLine -= 8;
        }

        value = io_inb(port) | (1 << irqLine);
        io_outb(port, value);
    }

    void clearMask(uint8_t irqLine) {
        uint16_t port;
        uint8_t value;

        if (irqLine < 8)
            port = PIC1_DATA;

        else
        {
            port = PIC2_DATA;
            irqLine -= 8;
        }

        value = io_inb(port) & ~(1 << irqLine);
        io_outb(port, value);
    }

    void signalEOI(uint64_t isrNumber) {
        if (isrNumber >= 40)				// if the IRQ came from the slave PIC
            io_outb(PIC2_COMMAND, 0x20);

        io_outb(PIC1_COMMAND, 0x20);		// if the IRQ came from the master and/or the slave PIC
    }
}