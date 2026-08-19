#pragma once

#include "keycodes.h"

// PIN and KEY enums come from variant.h (provided by the board)
// SAMPLE_RATE is KSM_SAMPLE_RATE from variant.h

namespace ksm {
	void setColor(int key, int color);
	void setRGB(int key, float r, float g, float b);
	void setHSV(int key, float h, float s, float v);
	bool down(int key);
	bool press(int key);
	bool release(int key);
	bool hold(int key, int ms);
	void setRumble(bool on);
	long long getMicroTime();
	double getTime();
	void setupAudio(void (*callback)(int16_t*, int));
	void init();
	void loop();
    int getBatLevel();

	// Keyboard HID API
	//
	// Pick a transport in setup() — initKeyboardUSB(), initKeyboardBLE(), or
	// both — then drive keys with setKeyboard(). Each init owns its side of
	// the plumbing (descriptors, HID service, report delivery), so a sketch
	// never has to touch TinyUSB or Bluefruit directly.
	//
	//   void setup() {
	//     ksm::init();
	//     ksm::initKeyboardUSB();
	//   }

	// Enumerate as a USB HID keyboard.
	void initKeyboardUSB();

	// Advertise as a BLE HID keyboard, pairable as `name`.
	void initKeyboardBLE(const char* name = "KSM1");

	// True once some transport is connected and ready for reports. With both
	// transports up, either one being ready is enough.
	bool keyboardConnected();

	void setKeyboard(int key, bool down);
	void clearKeyboard();

	// Escape hatch for custom HID: receives the assembled boot-keyboard
	// report instead of (not in addition to) the built-in transports. Only
	// needed if you're building your own descriptor; initKeyboardUSB() and
	// initKeyboardBLE() are the normal path.
	void setKeyboardReportCallback(void (*callback)(uint8_t modifiers, uint8_t* keys));
}
