#include <Arduino.h>

constexpr int pinId(int r, int i) {
	return r * 32 + i;
}

enum PIN {
    PIN_MENU		= pinId(1,10),
    PIN_BAT_LVL 	= pinId(0,29),
    PIN_USB_ST 		= pinId(0,31),
    PIN_CHG 		= pinId(0,30),
	PIN_K10 		= pinId(0,27),
    PIN_LED 		= pinId(0,0),
    PIN_I2S_LRCK 	= pinId(0,1),
    PIN_I2S_DIN 	= pinId(0,26),
	PIN_I2C_SDA 	= pinId(0,4),
    PIN_I2S_BCK 	= pinId(0,5),
	PIN_K5 			= pinId(0,6),
    PIN_PWR_LED 	= pinId(0,7),
    PIN_K4 			= pinId(0,8),
    PIN_K9 			= pinId(1,9),
	PIN_I2C_SCL 	= pinId(0,11),
    PIN_GYRO_PWR 	= pinId(0,12),
    PIN_PWR_ON 		= pinId(0,25),
    PIN_K6 			= pinId(0,24),
    PIN_K1 			= pinId(0,22),
    PIN_K7 			= pinId(0,20),
    PIN_VIB 		= pinId(0,21),
    PIN_K2 			= pinId(0,17),
    PIN_K8 			= pinId(0,15),
    PIN_K3 			= pinId(0,13),
};
PIN KEY_PINS[] = { 
	PIN_MENU, PIN_K1, PIN_K2, PIN_K3, PIN_K4, PIN_K5, PIN_K6, PIN_K7, PIN_K8, PIN_K9, PIN_K10
};

enum KEY {
	KEY_MENU = 0,
	KEY_1 = 1,
	KEY_2 = 2,
	KEY_3 = 3,
	KEY_4 = 4,
	KEY_5 = 5,
	KEY_6 = 6,
	KEY_7 = 7,
	KEY_8 = 8,
	KEY_9 = 9,
	KEY_10 = 10,
	KEY_COUNT = 11,
};

#include <LSM6DS3.h>


#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels(10, PIN_LED, NEO_GRB + NEO_KHZ800);
int PIX_INDICES[] = { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

int keyStates[KEY_COUNT];
int pixelsBuffer[10];
bool flushPixels = false;


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

int64_t timeOffset = 0;
uint32_t lastTime = 0;
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

void setup() {
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

void readKeys() {
	for(int i=0; i<=10; ++i) {
		int idx = KEY_PINS[i];
		keyStates[i] = digitalRead(idx) == LOW;
	}
	// keyStates[0] = digitalRead(PIN_MENU) == LOW;
}

void loop() {
	readKeys();
	// TinyUSB_Device_FlushCDC();
	// pixels.fill(0x000030);
	// pixels.show();

	// delay(300);

	// pixels.fill(0x000000);
	// pixels.show();

	// delay(300);


	for(int i=0; i<KEY_COUNT; ++i) {
		if(isDown(i)) {
			setColor(i, 0x200000);
		} else {
			setColor(i, 0);
		}
	}
	
	if(flushPixels) {
		for(int i=0; i<10; ++i)
			pixels.setPixelColor(i, pixelsBuffer[i]);
		pixels.show();
		flushPixels = false;
	}
	delay(1);
}
