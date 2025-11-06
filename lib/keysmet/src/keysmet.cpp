#include "keysmet.h"

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LSM6DS3.h>

// Key pins array (must be outside anonymous namespace for external access if needed)
static const PIN KEY_PINS[] = { 
	PIN_MENU, PIN_K1, PIN_K2, PIN_K3, PIN_K4, PIN_K5, PIN_K6, PIN_K7, PIN_K8, PIN_K9, PIN_K10
};

namespace {
// Internal implementation details - not accessible outside this file

Adafruit_NeoPixel pixels(10, PIN_LED, NEO_GRB + NEO_KHZ800);
int PIX_INDICES[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

// State arrays
int keyStates[KEY_COUNT];
int pixelsBuffer[10];
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
    for(int i=0; i<=10; ++i) {
        int idx = KEY_PINS[i];
        keyStates[i] = digitalRead(idx) == LOW;
    }
}

} // end anonymous namespace

// Public API Functions - accessible from main.cpp
void setColor(int key, int color) {
	if(key < 1 || key > 10) return;
	pixelsBuffer[PIX_INDICES[key-1]] = color;
	flushPixels = true;
}

void setRGB(int key, int r, int g, int b) {
	setColor(key, pixels.Color(r, g, b));
}

void setHSV(int key, int h, int s, int v) {
	setColor(key, pixels.ColorHSV(h, s, v));
}

bool isDown(int key) {
	if(key < 0 || key >= KEY_COUNT)
		return false;
	return keyStates[key];
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
		for(int i=0; i<10; ++i)
			pixels.setPixelColor(i, pixelsBuffer[i]);
		pixels.show();
		flushPixels = false;
	}
    
    delay(1);
    readKeys();
}
