#pragma once

#include <stdint.h>

// See TinyUSB hid.h

namespace ksm {
namespace keycodes {
static const uint8_t NONE = 0x00;
static const uint8_t A = 0x04;
static const uint8_t B = 0x05;
static const uint8_t C = 0x06;
static const uint8_t D = 0x07;
static const uint8_t E = 0x08;
static const uint8_t F = 0x09;
static const uint8_t G = 0x0A;
static const uint8_t H = 0x0B;
static const uint8_t I = 0x0C;
static const uint8_t J = 0x0D;
static const uint8_t K = 0x0E;
static const uint8_t L = 0x0F;
static const uint8_t M = 0x10;
static const uint8_t N = 0x11;
static const uint8_t O = 0x12;
static const uint8_t P = 0x13;
static const uint8_t Q = 0x14;
static const uint8_t R = 0x15;
static const uint8_t S = 0x16;
static const uint8_t T = 0x17;
static const uint8_t U = 0x18;
static const uint8_t V = 0x19;
static const uint8_t W = 0x1A;
static const uint8_t X = 0x1B;
static const uint8_t Y = 0x1C;
static const uint8_t Z = 0x1D;
static const uint8_t NUM_1 = 0x1E;
static const uint8_t NUM_2 = 0x1F;
static const uint8_t NUM_3 = 0x20;
static const uint8_t NUM_4 = 0x21;
static const uint8_t NUM_5 = 0x22;
static const uint8_t NUM_6 = 0x23;
static const uint8_t NUM_7 = 0x24;
static const uint8_t NUM_8 = 0x25;
static const uint8_t NUM_9 = 0x26;
static const uint8_t NUM_0 = 0x27;
static const uint8_t ENTER = 0x28;
static const uint8_t ESCAPE = 0x29;
static const uint8_t BACKSPACE = 0x2A;
static const uint8_t TAB = 0x2B;
static const uint8_t SPACE = 0x2C;
static const uint8_t MINUS = 0x2D;
static const uint8_t EQUAL = 0x2E;
static const uint8_t BRACKET_LEFT = 0x2F;
static const uint8_t BRACKET_RIGHT = 0x30;
static const uint8_t BACKSLASH = 0x31;
static const uint8_t EUROPE_1 = 0x32;
static const uint8_t SEMICOLON = 0x33;
static const uint8_t APOSTROPHE = 0x34;
static const uint8_t GRAVE = 0x35;
static const uint8_t COMMA = 0x36;
static const uint8_t PERIOD = 0x37;
static const uint8_t SLASH = 0x38;
static const uint8_t CAPS_LOCK = 0x39;
static const uint8_t F1 = 0x3A;
static const uint8_t F2 = 0x3B;
static const uint8_t F3 = 0x3C;
static const uint8_t F4 = 0x3D;
static const uint8_t F5 = 0x3E;
static const uint8_t F6 = 0x3F;
static const uint8_t F7 = 0x40;
static const uint8_t F8 = 0x41;
static const uint8_t F9 = 0x42;
static const uint8_t F10 = 0x43;
static const uint8_t F11 = 0x44;
static const uint8_t F12 = 0x45;
static const uint8_t PRINT_SCREEN = 0x46;
static const uint8_t SCROLL_LOCK = 0x47;
static const uint8_t PAUSE = 0x48;
static const uint8_t INSERT = 0x49;
static const uint8_t HOME = 0x4A;
static const uint8_t PAGE_UP = 0x4B;
static const uint8_t DELETE = 0x4C;
static const uint8_t END = 0x4D;
static const uint8_t PAGE_DOWN = 0x4E;
static const uint8_t ARROW_RIGHT = 0x4F;
static const uint8_t ARROW_LEFT = 0x50;
static const uint8_t ARROW_DOWN = 0x51;
static const uint8_t ARROW_UP = 0x52;
static const uint8_t NUM_LOCK = 0x53;
static const uint8_t KEYPAD_DIVIDE = 0x54;
static const uint8_t KEYPAD_MULTIPLY = 0x55;
static const uint8_t KEYPAD_SUBTRACT = 0x56;
static const uint8_t KEYPAD_ADD = 0x57;
static const uint8_t KEYPAD_ENTER = 0x58;
static const uint8_t KEYPAD_1 = 0x59;
static const uint8_t KEYPAD_2 = 0x5A;
static const uint8_t KEYPAD_3 = 0x5B;
static const uint8_t KEYPAD_4 = 0x5C;
static const uint8_t KEYPAD_5 = 0x5D;
static const uint8_t KEYPAD_6 = 0x5E;
static const uint8_t KEYPAD_7 = 0x5F;
static const uint8_t KEYPAD_8 = 0x60;
static const uint8_t KEYPAD_9 = 0x61;
static const uint8_t KEYPAD_0 = 0x62;
static const uint8_t KEYPAD_DECIMAL = 0x63;
static const uint8_t EUROPE_2 = 0x64;
static const uint8_t APPLICATION = 0x65;
static const uint8_t POWER = 0x66;
static const uint8_t KEYPAD_EQUAL = 0x67;
static const uint8_t F13 = 0x68;
static const uint8_t F14 = 0x69;
static const uint8_t F15 = 0x6A;
static const uint8_t F16 = 0x6B;
static const uint8_t F17 = 0x6C;
static const uint8_t F18 = 0x6D;
static const uint8_t F19 = 0x6E;
static const uint8_t F20 = 0x6F;
static const uint8_t F21 = 0x70;
static const uint8_t F22 = 0x71;
static const uint8_t F23 = 0x72;
static const uint8_t F24 = 0x73;
static const uint8_t EXECUTE = 0x74;
static const uint8_t HELP = 0x75;
static const uint8_t MENU = 0x76;
static const uint8_t SELECT = 0x77;
static const uint8_t STOP = 0x78;
static const uint8_t AGAIN = 0x79;
static const uint8_t UNDO = 0x7A;
static const uint8_t CUT = 0x7B;
static const uint8_t COPY = 0x7C;
static const uint8_t PASTE = 0x7D;
static const uint8_t FIND = 0x7E;
static const uint8_t MUTE = 0x7F;
static const uint8_t VOLUME_UP = 0x80;
static const uint8_t VOLUME_DOWN = 0x81;
static const uint8_t LOCKING_CAPS_LOCK = 0x82;
static const uint8_t LOCKING_NUM_LOCK = 0x83;
static const uint8_t LOCKING_SCROLL_LOCK = 0x84;
static const uint8_t KEYPAD_COMMA = 0x85;
static const uint8_t KEYPAD_EQUAL_SIGN = 0x86;
static const uint8_t KANJI1 = 0x87;
static const uint8_t KANJI2 = 0x88;
static const uint8_t KANJI3 = 0x89;
static const uint8_t KANJI4 = 0x8A;
static const uint8_t KANJI5 = 0x8B;
static const uint8_t KANJI6 = 0x8C;
static const uint8_t KANJI7 = 0x8D;
static const uint8_t KANJI8 = 0x8E;
static const uint8_t KANJI9 = 0x8F;
static const uint8_t LANG1 = 0x90;
static const uint8_t LANG2 = 0x91;
static const uint8_t LANG3 = 0x92;
static const uint8_t LANG4 = 0x93;
static const uint8_t LANG5 = 0x94;
static const uint8_t LANG6 = 0x95;
static const uint8_t LANG7 = 0x96;
static const uint8_t LANG8 = 0x97;
static const uint8_t LANG9 = 0x98;
static const uint8_t ALTERNATE_ERASE = 0x99;
static const uint8_t SYSREQ_ATTENTION = 0x9A;
static const uint8_t CANCEL = 0x9B;
static const uint8_t CLEAR = 0x9C;
static const uint8_t PRIOR = 0x9D;
static const uint8_t RETURN = 0x9E;
static const uint8_t SEPARATOR = 0x9F;
static const uint8_t OUT = 0xA0;
static const uint8_t OPER = 0xA1;
static const uint8_t CLEAR_AGAIN = 0xA2;
static const uint8_t CRSEL_PROPS = 0xA3;
static const uint8_t EXSEL = 0xA4;

static const uint8_t CONTROL_LEFT = 0xE0;
static const uint8_t SHIFT_LEFT = 0xE1;
static const uint8_t ALT_LEFT = 0xE2;
static const uint8_t GUI_LEFT = 0xE3;
static const uint8_t CONTROL_RIGHT = 0xE4;
static const uint8_t SHIFT_RIGHT = 0xE5;
static const uint8_t ALT_RIGHT = 0xE6;
static const uint8_t GUI_RIGHT = 0xE7;


static const uint8_t MODIFIER_LEFTCTRL   = (1 << 0);  // Left Control
static const uint8_t MODIFIER_LEFTSHIFT  = (1 << 1);  // Left Shift
static const uint8_t MODIFIER_LEFTALT    = (1 << 2);  // Left Alt
static const uint8_t MODIFIER_LEFTGUI    = (1 << 3);  // Left Window
static const uint8_t MODIFIER_RIGHTCTRL  = (1 << 4);  // Right Control
static const uint8_t MODIFIER_RIGHTSHIFT = (1 << 5);  // Right Shift
static const uint8_t MODIFIER_RIGHTALT   = (1 << 6);  // Right Alt
static const uint8_t MODIFIER_RIGHTGUI   = (1 << 7);  // Right Window


// Consumer & media keys
static const uint16_t CONSUMER_FLAG                              = 0x8000;

// Power Control
static const uint16_t CONSUMER_POWER                             = 0x0030 | CONSUMER_FLAG;
static const uint16_t CONSUMER_RESET                             = 0x0031 | CONSUMER_FLAG;
static const uint16_t CONSUMER_SLEEP                             = 0x0032 | CONSUMER_FLAG;

// Screen Brightness
static const uint16_t CONSUMER_BRIGHTNESS_INCREMENT              = 0x006F | CONSUMER_FLAG;
static const uint16_t CONSUMER_BRIGHTNESS_DECREMENT              = 0x0070 | CONSUMER_FLAG;

// Media Control
static const uint16_t CONSUMER_PLAY_PAUSE                        = 0x00CD | CONSUMER_FLAG;
static const uint16_t CONSUMER_SCAN_NEXT                         = 0x00B5 | CONSUMER_FLAG;
static const uint16_t CONSUMER_SCAN_PREVIOUS                     = 0x00B6 | CONSUMER_FLAG;
static const uint16_t CONSUMER_STOP                              = 0x00B7 | CONSUMER_FLAG;
static const uint16_t CONSUMER_VOLUME                            = 0x00E0 | CONSUMER_FLAG;
static const uint16_t CONSUMER_MUTE                              = 0x00E2 | CONSUMER_FLAG;
static const uint16_t CONSUMER_BASS                              = 0x00E3 | CONSUMER_FLAG;
static const uint16_t CONSUMER_TREBLE                            = 0x00E4 | CONSUMER_FLAG;
static const uint16_t CONSUMER_BASS_BOOST                        = 0x00E5 | CONSUMER_FLAG;
static const uint16_t CONSUMER_VOLUME_INCREMENT                  = 0x00E9 | CONSUMER_FLAG;
static const uint16_t CONSUMER_VOLUME_DECREMENT                  = 0x00EA | CONSUMER_FLAG;
static const uint16_t CONSUMER_BASS_INCREMENT                    = 0x0152 | CONSUMER_FLAG;
static const uint16_t CONSUMER_BASS_DECREMENT                    = 0x0153 | CONSUMER_FLAG;
static const uint16_t CONSUMER_TREBLE_INCREMENT                  = 0x0154 | CONSUMER_FLAG;
static const uint16_t CONSUMER_TREBLE_DECREMENT                  = 0x0155 | CONSUMER_FLAG;

}; // namespace keycodes

}; // namespace ksm