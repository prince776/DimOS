#include <kernel/devices/keyboard.h>
#include <kernel/devices/pic.h>
#include <kernel/filesystem/vfs.h>
#include <kernel/process/kthread.h>
#include <kernel/x64.h>
#include <stdio.h>

extern vfs::VFS globalVFS;

Keyboard::KeyCode Keyboard::getLastKeyCode() {
    if (scancode == INVALID_SCANCODE) {
        return KeyCode::KEY_UNKNOWN;
    }
    return scancodeToKeyCode[scancode];
}

void Keyboard::discardLastKeyCode() { scancode = INVALID_SCANCODE; }

void Keyboard::install() {
    pic::clearMask(IRQLine);

    registerInterruptHandler(pic::PIC1Offset + IRQLine, Keyboard::isrHandler);
}

uint8_t Keyboard::readControllerStatus() { return io_inb(ControllerIO::CTRL_STATS_REG); }

void Keyboard::sendControllerCommand(uint8_t cmd) {
    while (true) {
        if ((readControllerStatus() & CTRL_STATS_MASK_IN_BUF) == 0) {
            break;
        }
    }

    io_outb(CTRL_CMD_REG, cmd);
}

uint8_t Keyboard::readEncoderBuffer() { return io_inb(ENC_INPUT_BUF); }

void Keyboard::sendEncoderCommand(uint8_t cmd) {
    while (true) {
        if ((readControllerStatus() & CTRL_STATS_MASK_IN_BUF) == 0) {
            break;
        }
    }

    io_outb(ENC_CMD_REG, cmd);
}

// On keyboard event, interrupt happens. After that we have to interact with device to get stuff.
void Keyboard::isrHandler(ISRFrame* frame) {
    static bool _extended = false;
    auto& device = get();

    int code = 0;
    // read scan code only if the kkybrd controller output buffer is full (scan code is in it)
    if (device.readControllerStatus() & CTRL_STATS_MASK_OUT_BUF) {

        // read the scan code
        code = device.readEncoderBuffer();

        // is this an extended code? If so, set it and return
        if (code == 0xE0 || code == 0xE1) {
            _extended = true;
        } else {

            // either the second byte of an extended scan code or a single byte scan code
            _extended = false;

            // test if this is a break code (Original XT Scan Code Set specific)
            if (code & 0x80) { // test bit 7

                // covert the break code into its make code equivelant
                code -= 0x80;

                // grab the key
                KeyCode key = Keyboard::scancodeToKeyCode[code];

                // test if a special key has been released & set it
                switch (key) {

                case KEY_LCTRL:
                case KEY_RCTRL:
                    device.ctrl = false;
                    break;

                case KEY_LSHIFT:
                case KEY_RSHIFT:
                    device.shift = false;
                    break;

                case KEY_LALT:
                case KEY_RALT:
                    device.alt = false;
                    break;
                }
            } else {

                // this is a make code - set the scan code
                device.scancode = code;

                // grab the key
                KeyCode key = Keyboard::scancodeToKeyCode[code];

                // test if user is holding down any special keys & set it
                switch (key) {

                case KEY_LCTRL:
                case KEY_RCTRL:
                    device.ctrl = true;
                    break;

                case KEY_LSHIFT:
                case KEY_RSHIFT:
                    device.shift = true;
                    break;

                case KEY_LALT:
                case KEY_RALT:
                    device.alt = true;
                    break;

                case KEY_CAPSLOCK:
                    device.capsLock = (device.capsLock) ? false : true;
                    break;

                case KEY_KP_NUMLOCK:
                    device.numLock = (device.numLock) ? false : true;
                    break;

                case KEY_SCROLLLOCK:
                    device.scrollLock = (device.scrollLock) ? false : true;
                    break;
                }
            }
        }
        device.error = 0;
        // watch for errors
        switch (code) {
        case ERR_BAT_FAILED:
        case ERR_DIAG_FAILED:
        case ERR_RESEND_CMD:
            device.error = code;
            break;
        }
    }

    // TODO: ok so here we are writing to a file, that is we need concurrency, but acquiring lock in
    // isr may not work, just a reminder.
    auto key = device.getLastKeyCode();
    if (key != KEY_UNKNOWN) {
        auto& stdinFileDescriptor = kernel::thisThread().fileDescriptors[0];
        stdinFileDescriptor.write(sizeof(key), (uint8_t*)&key);
    }
    device.discardLastKeyCode();
}

static bool isascii(char c) { return ((unsigned)(c) <= 0x7F); }

char Keyboard::keyCodeToASCII(KeyCode code) {

    uint8_t key = code;

    // insure key is an ascii character
    if (isascii(key)) {

        // if shift key is down or caps lock is on, make the key uppercase
        if (shift || capsLock)
            if (key >= 'a' && key <= 'z')
                key -= 32;

        if (shift && !capsLock)
            if (key >= '0' && key <= '9')
                switch (key) {

                case '0':
                    key = KEY_RIGHTPARENTHESIS;
                    break;
                case '1':
                    key = KEY_EXCLAMATION;
                    break;
                case '2':
                    key = KEY_AT;
                    break;
                case '3':
                    key = KEY_EXCLAMATION;
                    break;
                case '4':
                    key = KEY_HASH;
                    break;
                case '5':
                    key = KEY_PERCENT;
                    break;
                case '6':
                    key = KEY_CARRET;
                    break;
                case '7':
                    key = KEY_AMPERSAND;
                    break;
                case '8':
                    key = KEY_ASTERISK;
                    break;
                case '9':
                    key = KEY_LEFTPARENTHESIS;
                    break;
                }
            else {

                switch (key) {
                case KEY_COMMA:
                    key = KEY_LESS;
                    break;

                case KEY_DOT:
                    key = KEY_GREATER;
                    break;

                case KEY_SLASH:
                    key = KEY_QUESTION;
                    break;

                case KEY_SEMICOLON:
                    key = KEY_COLON;
                    break;

                case KEY_QUOTE:
                    key = KEY_QUOTEDOUBLE;
                    break;

                case KEY_LEFTBRACKET:
                    key = KEY_LEFTCURL;
                    break;

                case KEY_RIGHTBRACKET:
                    key = KEY_RIGHTCURL;
                    break;

                case KEY_GRAVE:
                    key = KEY_TILDE;
                    break;

                case KEY_MINUS:
                    key = KEY_UNDERSCORE;
                    break;

                case KEY_PLUS:
                    key = KEY_EQUAL;
                    break;

                case KEY_BACKSLASH:
                    key = KEY_BAR;
                    break;
                }
            }

        // return the key
        return key;
    }

    // scan code != a valid ascii char so no convertion is possible
    return 0;
}
