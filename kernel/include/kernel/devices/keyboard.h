#pragma once
#include <kernel/isr.h>
#include <stdint.h>

// Big part of this copied from BrokenThorn tutorial.
class Keyboard {
  private:
    Keyboard()
        : scancode(INVALID_SCANCODE), numLock(false), scrollLock(false), capsLock(false),
          shift(false), alt(false), ctrl(false), error(0) {}

    Keyboard(const Keyboard&) = delete;
    void operator=(const Keyboard&) = delete;

  public:
    static Keyboard& get() {
        static Keyboard device{};
        return device;
    }
    static void isrHandler(ISRFrame* frame);

    bool isScrollLock() { return scrollLock; }
    bool isCapsLock() { return capsLock; }
    bool isNumLock() { return numLock; }

    bool isAlt() { return alt; }
    bool isCtrl() { return ctrl; }
    bool isShift() { return shift; }

    bool getError() { return error; }

    enum KeyCode : int;

    uint8_t getLastScanCode() { return scancode; }

    KeyCode getLastKeyCode();
    void discardLastKeyCode();

    void install();

    char keyCodeToASCII(KeyCode);

    static constexpr int IRQLine = 1;

  private:
    char scancode{INVALID_SCANCODE};
    bool numLock{}, scrollLock{}, capsLock{};
    bool shift{}, alt{}, ctrl{};
    int error{};

    uint8_t readControllerStatus();
    void sendControllerCommand(uint8_t);

    uint8_t readEncoderBuffer();
    void sendEncoderCommand(uint8_t);

  private:
    enum EncoderIO { ENC_INPUT_BUF = 0x60, ENC_CMD_REG = 0x60 };

    enum EncoderCmd {
        ENC_CMD_SET_LED = 0xED,
        ENC_CMD_ECHO = 0xEE,
        ENC_CMD_SCAN_CODE_SET = 0xF0,
        ENC_CMD_ID = 0xF2,
        ENC_CMD_AUTODELAY = 0xF3,
        ENC_CMD_ENABLE = 0xF4,
        ENC_CMD_RESETWAIT = 0xF5,
        ENC_CMD_RESETSCAN = 0xF6,
        ENC_CMD_ALL_AUTO = 0xF7,
        ENC_CMD_ALL_MAKEBREAK = 0xF8,
        ENC_CMD_ALL_MAKEONLY = 0xF9,
        ENC_CMD_ALL_MAKEBREAK_AUTO = 0xFA,
        ENC_CMD_SINGLE_AUTOREPEAT = 0xFB,
        ENC_CMD_SINGLE_MAKEBREAK = 0xFC,
        ENC_CMD_SINGLE_BREAKONLY = 0xFD,
        ENC_CMD_RESEND = 0xFE,
        ENC_CMD_RESET = 0xFF
    };

    enum ControllerIO { CTRL_STATS_REG = 0x64, CTRL_CMD_REG = 0x64 };

    enum ControllerStatsMask {
        CTRL_STATS_MASK_OUT_BUF = 1,    // 00000001
        CTRL_STATS_MASK_IN_BUF = 2,     // 00000010
        CTRL_STATS_MASK_SYSTEM = 4,     // 00000100
        CTRL_STATS_MASK_CMD_DATA = 8,   // 00001000
        CTRL_STATS_MASK_LOCKED = 0x10,  // 00010000
        CTRL_STATS_MASK_AUX_BUF = 0x20, // 00100000
        CTRL_STATS_MASK_TIMEOUT = 0x40, // 01000000
        CTRL_STATS_MASK_PARITY = 0x80   // 10000000
    };

    enum ControllerCmd {
        CTRL_CMD_READ = 0x20,
        CTRL_CMD_WRITE = 0x60,
        CTRL_CMD_SELF_TEST = 0xAA,
        CTRL_CMD_INTERFACE_TEST = 0xAB,
        CTRL_CMD_DISABLE = 0xAD,
        CTRL_CMD_ENABLE = 0xAE,
        CTRL_CMD_READ_IN_PORT = 0xC0,
        CTRL_CMD_READ_OUT_PORT = 0xD0,
        CTRL_CMD_WRITE_OUT_PORT = 0xD1,
        CTRL_CMD_READ_TEST_INPUTS = 0xE0,
        CTRL_CMD_SYSTEM_RESET = 0xFE,
        CTRL_CMD_MOUSE_DISABLE = 0xA7,
        CTRL_CMD_MOUSE_ENABLE = 0xA8,
        CTRL_CMD_MOUSE_PORT_TEST = 0xA9,
        CTRL_CMD_MOUSE_WRITE = 0xD4
    };

    enum Error {
        ERR_BUF_OVERRUN = 0,
        ERR_ID_RET = 0x83AB,
        ERR_BAT = 0xAA, // note: can also be L. shift key make code
        ERR_ECHO_RET = 0xEE,
        ERR_ACK = 0xFA,
        ERR_BAT_FAILED = 0xFC,
        ERR_DIAG_FAILED = 0xFD,
        ERR_RESEND_CMD = 0xFE,
        ERR_KEY = 0xFF
    };

  public:
    enum KeyCode : int {
        // Alphanumeric keys ////////////////

        KEY_SPACE = ' ',
        KEY_0 = '0',
        KEY_1 = '1',
        KEY_2 = '2',
        KEY_3 = '3',
        KEY_4 = '4',
        KEY_5 = '5',
        KEY_6 = '6',
        KEY_7 = '7',
        KEY_8 = '8',
        KEY_9 = '9',

        KEY_A = 'a',
        KEY_B = 'b',
        KEY_C = 'c',
        KEY_D = 'd',
        KEY_E = 'e',
        KEY_F = 'f',
        KEY_G = 'g',
        KEY_H = 'h',
        KEY_I = 'i',
        KEY_J = 'j',
        KEY_K = 'k',
        KEY_L = 'l',
        KEY_M = 'm',
        KEY_N = 'n',
        KEY_O = 'o',
        KEY_P = 'p',
        KEY_Q = 'q',
        KEY_R = 'r',
        KEY_S = 's',
        KEY_T = 't',
        KEY_U = 'u',
        KEY_V = 'v',
        KEY_W = 'w',
        KEY_X = 'x',
        KEY_Y = 'y',
        KEY_Z = 'z',

        KEY_RETURN = '\r',
        KEY_ESCAPE = 0x1001,
        KEY_BACKSPACE = '\b',

        // Arrow keys ////////////////////////

        KEY_UP = 0x1100,
        KEY_DOWN = 0x1101,
        KEY_LEFT = 0x1102,
        KEY_RIGHT = 0x1103,

        // Function keys /////////////////////

        KEY_F1 = 0x1201,
        KEY_F2 = 0x1202,
        KEY_F3 = 0x1203,
        KEY_F4 = 0x1204,
        KEY_F5 = 0x1205,
        KEY_F6 = 0x1206,
        KEY_F7 = 0x1207,
        KEY_F8 = 0x1208,
        KEY_F9 = 0x1209,
        KEY_F10 = 0x120a,
        KEY_F11 = 0x120b,
        KEY_F12 = 0x120b,
        KEY_F13 = 0x120c,
        KEY_F14 = 0x120d,
        KEY_F15 = 0x120e,

        KEY_DOT = '.',
        KEY_COMMA = ',',
        KEY_COLON = ':',
        KEY_SEMICOLON = ';',
        KEY_SLASH = '/',
        KEY_BACKSLASH = '\\',
        KEY_PLUS = '+',
        KEY_MINUS = '-',
        KEY_ASTERISK = '*',
        KEY_EXCLAMATION = '!',
        KEY_QUESTION = '?',
        KEY_QUOTEDOUBLE = '\"',
        KEY_QUOTE = '\'',
        KEY_EQUAL = '=',
        KEY_HASH = '#',
        KEY_PERCENT = '%',
        KEY_AMPERSAND = '&',
        KEY_UNDERSCORE = '_',
        KEY_LEFTPARENTHESIS = '(',
        KEY_RIGHTPARENTHESIS = ')',
        KEY_LEFTBRACKET = '[',
        KEY_RIGHTBRACKET = ']',
        KEY_LEFTCURL = '{',
        KEY_RIGHTCURL = '}',
        KEY_DOLLAR = '$',
        KEY_EURO = '$',
        KEY_LESS = '<',
        KEY_GREATER = '>',
        KEY_BAR = '|',
        KEY_GRAVE = '`',
        KEY_TILDE = '~',
        KEY_AT = '@',
        KEY_CARRET = '^',

        // Numeric keypad //////////////////////

        KEY_KP_0 = '0',
        KEY_KP_1 = '1',
        KEY_KP_2 = '2',
        KEY_KP_3 = '3',
        KEY_KP_4 = '4',
        KEY_KP_5 = '5',
        KEY_KP_6 = '6',
        KEY_KP_7 = '7',
        KEY_KP_8 = '8',
        KEY_KP_9 = '9',
        KEY_KP_PLUS = '+',
        KEY_KP_MINUS = '-',
        KEY_KP_DECIMAL = '.',
        KEY_KP_DIVIDE = '/',
        KEY_KP_ASTERISK = '*',
        KEY_KP_NUMLOCK = 0x300f,
        KEY_KP_ENTER = 0x3010,

        KEY_TAB = 0x4000,
        KEY_CAPSLOCK = 0x4001,

        // Modify keys ////////////////////////////

        KEY_LSHIFT = 0x4002,
        KEY_LCTRL = 0x4003,
        KEY_LALT = 0x4004,
        KEY_LWIN = 0x4005,
        KEY_RSHIFT = 0x4006,
        KEY_RCTRL = 0x4007,
        KEY_RALT = 0x4008,
        KEY_RWIN = 0x4009,

        KEY_INSERT = 0x400a,
        KEY_DELETE = 0x400b,
        KEY_HOME = 0x400c,
        KEY_END = 0x400d,
        KEY_PAGEUP = 0x400e,
        KEY_PAGEDOWN = 0x400f,
        KEY_SCROLLLOCK = 0x4010,
        KEY_PAUSE = 0x4011,

        KEY_UNKNOWN,
        KEY_NUMKEYCODES
    };

    // Used to indicate the last scan code is not to be reused.
    constexpr static int INVALID_SCANCODE = 0;

  private:
    static constexpr KeyCode scancodeToKeyCode[] = {
        //! key			scancode
        KEY_UNKNOWN,      // 0
        KEY_ESCAPE,       // 1
        KEY_1,            // 2
        KEY_2,            // 3
        KEY_3,            // 4
        KEY_4,            // 5
        KEY_5,            // 6
        KEY_6,            // 7
        KEY_7,            // 8
        KEY_8,            // 9
        KEY_9,            // 0xa
        KEY_0,            // 0xb
        KEY_MINUS,        // 0xc
        KEY_EQUAL,        // 0xd
        KEY_BACKSPACE,    // 0xe
        KEY_TAB,          // 0xf
        KEY_Q,            // 0x10
        KEY_W,            // 0x11
        KEY_E,            // 0x12
        KEY_R,            // 0x13
        KEY_T,            // 0x14
        KEY_Y,            // 0x15
        KEY_U,            // 0x16
        KEY_I,            // 0x17
        KEY_O,            // 0x18
        KEY_P,            // 0x19
        KEY_LEFTBRACKET,  // 0x1a
        KEY_RIGHTBRACKET, // 0x1b
        KEY_RETURN,       // 0x1c
        KEY_LCTRL,        // 0x1d
        KEY_A,            // 0x1e
        KEY_S,            // 0x1f
        KEY_D,            // 0x20
        KEY_F,            // 0x21
        KEY_G,            // 0x22
        KEY_H,            // 0x23
        KEY_J,            // 0x24
        KEY_K,            // 0x25
        KEY_L,            // 0x26
        KEY_SEMICOLON,    // 0x27
        KEY_QUOTE,        // 0x28
        KEY_GRAVE,        // 0x29
        KEY_LSHIFT,       // 0x2a
        KEY_BACKSLASH,    // 0x2b
        KEY_Z,            // 0x2c
        KEY_X,            // 0x2d
        KEY_C,            // 0x2e
        KEY_V,            // 0x2f
        KEY_B,            // 0x30
        KEY_N,            // 0x31
        KEY_M,            // 0x32
        KEY_COMMA,        // 0x33
        KEY_DOT,          // 0x34
        KEY_SLASH,        // 0x35
        KEY_RSHIFT,       // 0x36
        KEY_KP_ASTERISK,  // 0x37
        KEY_RALT,         // 0x38
        KEY_SPACE,        // 0x39
        KEY_CAPSLOCK,     // 0x3a
        KEY_F1,           // 0x3b
        KEY_F2,           // 0x3c
        KEY_F3,           // 0x3d
        KEY_F4,           // 0x3e
        KEY_F5,           // 0x3f
        KEY_F6,           // 0x40
        KEY_F7,           // 0x41
        KEY_F8,           // 0x42
        KEY_F9,           // 0x43
        KEY_F10,          // 0x44
        KEY_KP_NUMLOCK,   // 0x45
        KEY_SCROLLLOCK,   // 0x46
        KEY_HOME,         // 0x47
        KEY_KP_8,         // 0x48	//keypad up arrow
        KEY_PAGEUP,       // 0x49
        KEY_KP_2,         // 0x50	//keypad down arrow
        KEY_KP_3,         // 0x51	//keypad page down
        KEY_KP_0,         // 0x52	//keypad insert key
        KEY_KP_DECIMAL,   // 0x53	//keypad delete key
        KEY_UNKNOWN,      // 0x54
        KEY_UNKNOWN,      // 0x55
        KEY_UNKNOWN,      // 0x56
        KEY_F11,          // 0x57
        KEY_F12           // 0x58
    };
};
