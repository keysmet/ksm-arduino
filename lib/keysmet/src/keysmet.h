#pragma once

#include <functional>
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
	void setupAudio(std::function<void(int16_t*, int)> callback);
	void init();
	void loop();
    int getBatLevel();

	// Keyboard HID API
	void setKeyboard(int key, bool down);
	void clearKeyboard();
	void setKeyboardReportCallback(std::function<void(uint8_t modifiers, uint8_t* keys)> callback);
}
