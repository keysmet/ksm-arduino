// Keysmet ONE — PlatformIO entry point.
//
// The examples are Arduino sketches in examples/<Name>/<Name>.ino. Those
// .ino files are the single source of truth: the Arduino IDE opens them
// directly, and this file simply pulls one in so PlatformIO builds it.
//
// Each sketch defines its own setup() and loop(), so exactly one #include
// below may be active at a time. Uncomment the one you want to build.

#include <Arduino.h>

// PlatformIO's dependency finder scans this file, not the .ino it pulls in,
// so the libraries the examples use are named here to get them linked.
#include <bluefruit.h>
#include <keysmet.h>
#include <sfxr.h>

// ── Choose an example ──────────────────────────────────────────────────────

// #include "../examples/Sine/Sine.ino"               // audio: sine wave, one note per key
// #include "../examples/Bytebeat/Bytebeat.ino"       // audio: one-line algorithmic music
// #include "../examples/Sfxr/Sfxr.ino"                  // audio: retro game sound effects
// #include "../examples/Colors/Colors.ino"           // LEDs: gradients, patterns, animations
// #include "../examples/BleKeyboard/BleKeyboard.ino" // BLE HID keyboard
#include "../examples/KeyboardPresets/KeyboardPresets.ino" // USB HID keyboard, MENU cycles layouts
