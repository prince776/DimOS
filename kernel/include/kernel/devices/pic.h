#pragma once

#include <stdint.h>

namespace pic {
inline constexpr int PIC1 = 0x20;              // I/O base address for master PIC
inline constexpr int PIC2 = 0xA0;              // I/O base address for slave PIC
inline constexpr int PIC1_COMMAND = PIC1;      // master PIC command port
inline constexpr int PIC1_DATA = (PIC1 + 1);   // master PIC data port
inline constexpr int PIC2_COMMAND = PIC2;      // slave PIC command port
inline constexpr int PIC2_DATA = (PIC2 + 1);   // slave PIC data port
inline constexpr int ICW4_8086 = 0x01;         // 8086/88 (MCS-80/85) mode
inline constexpr int PIC1Offset = 0x20;        // 8086/88 (MCS-80/85) mode
inline constexpr int PIC2Offset = 0x28;        // 8086/88 (MCS-80/85) mode
inline constexpr int PIC2End = PIC2Offset + 8; // 8086/88 (MCS-80/85) mode

void disable();
void remap();
void maskLine(uint8_t irqLine);
void clearMask(uint8_t irqLine);
extern "C" void signalEOI(uint64_t isrNumber);
} // namespace pic
