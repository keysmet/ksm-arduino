#pragma once

#include <functional>

#define PINID(r, i) ((r) * 32 + (i))

const float SAMPLE_RATE = 22727.27f;

enum PIN {
    PIN_MENU		= PINID(1,10),
    PIN_BAT_LVL 	= PINID(0,29),
    PIN_USB_ST 		= PINID(0,31),
    PIN_CHG 		= PINID(0,30),
	PIN_K10 		= PINID(0,27),
    PIN_LED 		= PINID(0,0),
    PIN_I2S_LRCK 	= PINID(0,1),
    PIN_I2S_DIN 	= PINID(0,26),
	PIN_I2C_SDA 	= PINID(0,4),
    PIN_I2S_BCK 	= PINID(0,5),
	PIN_K5 			= PINID(0,6),
    PIN_PWR_LED 	= PINID(0,7),
    PIN_K4 			= PINID(0,8),
    PIN_K9 			= PINID(1,9),
	PIN_I2C_SCL 	= PINID(0,11),
    PIN_GYRO_PWR 	= PINID(0,12), // Unused
    PIN_PWR_ON 		= PINID(0,25),
    PIN_K6 			= PINID(0,24),
    PIN_K1 			= PINID(0,22),
    PIN_K7 			= PINID(0,20),
    PIN_VIB 		= PINID(0,21),
    PIN_K2 			= PINID(0,17),
    PIN_K8 			= PINID(0,15),
    PIN_K3 			= PINID(0,13),
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

void ksm_init();
void ksm_loop();
