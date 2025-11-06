#include "keysmet.h"

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LSM6DS3.h>

// Key pins array (must be outside anonymous namespace for external access if needed)
static const PIN KEY_PINS[] = { 
	PIN_MENU, PIN_K1, PIN_K2, PIN_K3, PIN_K4, PIN_K5, PIN_K6, PIN_K7, PIN_K8, PIN_K9, PIN_K10
};

namespace {

class KeyState {
public:
    int color = 0;
    int64_t pressTime = -1;
    bool down = false;
    bool wasDown = false;
};

Adafruit_NeoPixel pixels(10, PIN_LED, NEO_GRB + NEO_KHZ800);
int PIX_INDICES[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

// State array for all keys
KeyState keys[KEY_COUNT];
bool flushPixels = false;

// Timing variables
int64_t timeOffset = 0;
uint32_t lastTime = 0;

// Internal helper functions
void setupPins() {
    pinMode(PIN_K1, INPUT_PULLUP);
    pinMode(PIN_K2, INPUT_PULLUP);
    pinMode(PIN_K3, INPUT_PULLUP);
    pinMode(PIN_K4, INPUT_PULLUP);
    pinMode(PIN_K5, INPUT_PULLUP);
    pinMode(PIN_K6, INPUT_PULLUP);
    pinMode(PIN_K7, INPUT_PULLUP);
    pinMode(PIN_K8, INPUT_PULLUP);
    pinMode(PIN_K9, INPUT_PULLUP);
    pinMode(PIN_K10, INPUT_PULLUP);
    pinMode(PIN_MENU, INPUT_PULLUP);

    pinMode(PIN_BAT_LVL, INPUT);
    pinMode(PIN_USB_ST, INPUT);
    pinMode(PIN_CHG, INPUT);

    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_PWR_LED, OUTPUT);
    pinMode(PIN_PWR_ON, OUTPUT_S0H1);
    pinMode(PIN_GYRO_PWR, OUTPUT);

    pinMode(LED_BLUE, OUTPUT);

    // Set the reset pin to P0.18
    if (((NRF_UICR->PSELRESET[0]) == 0xFFFFFFFF) && ((NRF_UICR->PSELRESET[1]) == 0xFFFFFFFF))
    {                                // if the two RESET registers are erased
        NRF_NVMC->CONFIG = 1;        // Write enable the UICR
        NRF_UICR->PSELRESET[0] = 18; // designate pin P0.18 as the RESET pin
        NRF_UICR->PSELRESET[1] = 18; // designate pin P0.18 as the RESET pin
        NRF_NVMC->CONFIG = 0;        // Put the UICR back into read-only mode.
    }
}

void readKeys() {
    for(int i=0; i<KEY_COUNT; ++i) {
        keys[i].wasDown = keys[i].down;
        keys[i].down = digitalRead(KEY_PINS[i]) == LOW;
        
        if(keys[i].down && !keys[i].wasDown) {
            keys[i].pressTime = getMicroTime();
        }
        if(!keys[i].down) {
            keys[i].pressTime = -1;
        }
    }
}

} // end anonymous namespace

// Public API Functions - accessible from main.cpp
void setColor(int key, int color) {
	// Key 0 (MENU) doesn't have a pixel, only keys 1-10 have pixels
	if(key < 1 || key > 10) return;
	keys[key].color = color;
	flushPixels = true;
}

void setRGB(int key, int r, int g, int b) {
	setColor(key, pixels.Color(r, g, b));
}

void setHSV(int key, int h, int s, int v) {
	setColor(key, pixels.ColorHSV(h, s, v));
}

bool down(int key) {
	if(key < 0 || key >= KEY_COUNT)
		return false;
	return keys[key].down;
}

bool press(int key) {
	if(key < 0 || key >= KEY_COUNT)
		return false;
	return keys[key].down && !keys[key].wasDown;
}

bool release(int key) {
	if(key < 0 || key >= KEY_COUNT)
		return false;
	return !keys[key].down && keys[key].wasDown;
}

bool hold(int key, int ms) {
	if(key < 0 || key >= KEY_COUNT)
		return false;
	if(keys[key].pressTime < 0)
		return false;
	int64_t elapsed = getMicroTime() - keys[key].pressTime;
	return elapsed >= (int64_t)ms * 1000;
}

void setRumble(bool on) {
	// TODO
}

int64_t getMicroTime() {
	uint32_t t = micros();
	if(t < lastTime)  // handle overflow
		timeOffset += (int64_t)(0xFFFFFFFF / 64);
	lastTime = t;
	return timeOffset + t;
}

double getTime() {  
	return double(getMicroTime())  / (1000.0 * 1000.0);
}


void ksm_init() {
    setupPins();
    digitalWrite(PIN_PWR_ON, HIGH);
    digitalWrite(PIN_GYRO_PWR, HIGH);

    delay(10);
 
    // setupI2S();

    // Wire.setPins(PIN::I2C_SDA, PIN::I2C_SCL);
    // Wire.begin();
    // Wire.setClock(100000); // 100 kHz standard speed

    Serial.begin(115200);
    IMU.begin();

    NRF_NVMC->ICACHECNF = 1;
    dwt_enable();
    
    pixels.begin();
    pixels.clear();

    for(int i=0; i<3; ++i) {
		pixels.fill(0x000030);
		pixels.show();
		delay(50);
		pixels.fill(0);
		pixels.show();
		delay(50);
	}
}

void ksm_loop() {
    if(flushPixels) {
		for(int i=1; i<=10; ++i) {
			int pixelIdx = PIX_INDICES[i-1];
			pixels.setPixelColor(pixelIdx, keys[i].color);
		}
		pixels.show();
		flushPixels = false;
	}
    
    delay(1);
    readKeys();
}
