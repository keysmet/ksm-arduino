#include "keysmet.h"

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

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
    }
}

int menuPressCount = 0;
int64_t lastMenuPressTime = 0;

bool checkMenuStreakPress() {
    const int STREAK_COUNT = 3;
    const int MAX_INTERVAL_MS = 200;
    
    if(press(KEY_MENU)) {
        int64_t currentTime = getMicroTime();
        int64_t elapsed = (menuPressCount > 0) ? (currentTime - lastMenuPressTime) / 1000 : 0;
        
        if(menuPressCount > 0 && elapsed > MAX_INTERVAL_MS) {
            menuPressCount = 1;
        } else {
            menuPressCount++;
        }
        
        lastMenuPressTime = currentTime;
        
        if(menuPressCount >= STREAK_COUNT) {
            menuPressCount = 0;
            return true;
        }
    }
    
    if(menuPressCount > 0) {
        int64_t elapsed = (getMicroTime() - lastMenuPressTime) / 1000;
        if(elapsed > MAX_INTERVAL_MS) {
            menuPressCount = 0;
        }
    }
    
    return false;
}

void resetBootloader() {
    pixels.fill(0x0000FF);
    pixels.show();
    delay(500);
    pixels.fill(0x0);
    pixels.show();

    NRF_POWER->GPREGRET = 0x57;
    NVIC_SystemReset();
}

const int I2S_BUFFER_SIZE = 256;
int16_t i2sbuffers[2][ I2S_BUFFER_SIZE<<1 ];
int curi2sBuff = 0;
std::function<void(int16_t*, int)> audioCallback = nullptr;

void updateI2S() {
	int16_t* ptr = &i2sbuffers[curi2sBuff][0];
	audioCallback(ptr, I2S_BUFFER_SIZE);
	NRF_I2S->TXD.PTR = (uint32_t)ptr;
	NRF_I2S->RXTXD.MAXCNT = I2S_BUFFER_SIZE;
	curi2sBuff = (curi2sBuff+1) % 2;
}

TaskHandle_t audioTaskHandle;
int audioBusyTicks = 0;
int audioTotalTicks = 0;
void audioTask(void* arg) {
	while(true) {
		vTaskDelay(ms2tick(1));
		if(NRF_I2S->EVENTS_TXPTRUPD) {
			updateI2S();
			audioBusyTicks++;
			NRF_I2S->EVENTS_TXPTRUPD = 0;
		}
		++audioTotalTicks;
	}
}

void setupI2S() {  
	NRF_I2S->CONFIG.TXEN = (I2S_CONFIG_TXEN_TXEN_ENABLE << I2S_CONFIG_TXEN_TXEN_Pos);
	NRF_I2S->CONFIG.MCKEN = (I2S_CONFIG_MCKEN_MCKEN_ENABLE << I2S_CONFIG_MCKEN_MCKEN_Pos);
	NRF_I2S->CONFIG.MCKFREQ = I2S_CONFIG_MCKFREQ_MCKFREQ_32MDIV11  << I2S_CONFIG_MCKFREQ_MCKFREQ_Pos;
	NRF_I2S->CONFIG.MODE = I2S_CONFIG_MODE_MODE_MASTER << I2S_CONFIG_MODE_MODE_Pos;
	NRF_I2S->CONFIG.SWIDTH = I2S_CONFIG_SWIDTH_SWIDTH_16BIT << I2S_CONFIG_SWIDTH_SWIDTH_Pos;
	NRF_I2S->CONFIG.ALIGN = I2S_CONFIG_ALIGN_ALIGN_LEFT << I2S_CONFIG_ALIGN_ALIGN_Pos;
	NRF_I2S->CONFIG.FORMAT = I2S_CONFIG_FORMAT_FORMAT_I2S << I2S_CONFIG_FORMAT_FORMAT_Pos;
	NRF_I2S->CONFIG.CHANNELS = I2S_CONFIG_CHANNELS_CHANNELS_STEREO << I2S_CONFIG_CHANNELS_CHANNELS_Pos;
    NRF_I2S->CONFIG.RATIO = I2S_CONFIG_RATIO_RATIO_128X << I2S_CONFIG_RATIO_RATIO_Pos;

	NRF_I2S->PSEL.LRCK = (PIN::PIN_I2S_LRCK << I2S_PSEL_LRCK_PIN_Pos);  // RCK = D2
	NRF_I2S->PSEL.SCK = (PIN::PIN_I2S_BCK << I2S_PSEL_SCK_PIN_Pos);    // BCL/BCK = D0
	NRF_I2S->PSEL.SDOUT = (PIN::PIN_I2S_DIN << I2S_PSEL_SDOUT_PIN_Pos);  // DIN = D1

	NRF_I2S->ENABLE = 1;
	NRF_I2S->TASKS_START = 1;

	xTaskCreate(audioTask, "audio", 256, NULL, TASK_PRIO_NORMAL, &audioTaskHandle);
}



void shutdownLoop() {
    pixels.fill(0xff0000);
    pixels.show();
    delay(500);
    pixels.fill(0x0);
    pixels.show();
    Serial.println("Shutdown initiated");
    
    // Wait for menu button to be released
    Serial.println("Waiting for button release...");
    while(down(KEY_MENU)) {
        delay(1);
        readKeys();
    }
    Serial.println("Button released");
    
    Serial.println("Disabling DWT");
    dwt_disable();
    
    // Turn off LEDs and peripherals to save power
    Serial.println("Turning off LEDs and peripherals");
    pixels.clear();
    pixels.show();

    // Stop I2S if it's running
    if(audioCallback != nullptr) {
        Serial.println("Stopping I2S");
        NRF_I2S->TASKS_STOP = 1;
        NRF_I2S->ENABLE = 0;
    }
    
    // Delete audio task if it exists
    if(audioTaskHandle != NULL) {
        Serial.println("Deleting audio task");
        vTaskDelete(audioTaskHandle);
        audioTaskHandle = NULL;
    }
    
    // Suspend the FreeRTOS scheduler to stop all tasks
    Serial.println("Suspending FreeRTOS scheduler");
    vTaskSuspendAll();

    // Not sure about that
    delay(1);

    digitalWrite(PIN_PWR_LED, LOW);
    digitalWrite(PIN_GYRO_PWR, LOW);
    digitalWrite(LED_BLUE, LOW);
    delay(1);
    digitalWrite(PIN_PWR_ON, LOW);
    delay(1);
    // NRF_POWER->TASKS_CONSTLAT = 1;
    // delay(1);
    NRF_POWER->TASKS_LOWPWR = 1;
    
    // Disable SysTick (FreeRTOS tick interrupt) to prevent CPU wake
    SysTick->CTRL = 0;
    
    // Configure menu button for sense (wake on low)
    // This allows the button press to generate an event that wakes the CPU
    NRF_GPIO->PIN_CNF[PIN_MENU] = (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos)
                                 | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
                                 | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                                 | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    
    // Enter low-power loop
    while(true) {
        // Clear any pending events
        __SEV();
        __WFE();
        
        // Put CPU to sleep - will wake on button press event
        __WFE();
        
        // Check if menu button is pressed (active low)
        if(digitalRead(PIN_MENU) == LOW) {
            // Small busy-wait for debounce (can't use delay() as scheduler is suspended)
            for(volatile int i = 0; i < 100000; i++);
            
            // Confirm button is still pressed
            if(digitalRead(PIN_MENU) == LOW) {
                // Button confirmed - reset system
                NVIC_SystemReset();
            }
        }
    }
}


} // end anonymous namespace

void setColor(int key, int color) {
	// Key 0 (MENU) doesn't have a pixel, only keys 1-10 have pixels
	if(key < 1 || key > 10) return;
	keys[key].color = color;
	flushPixels = true;
}

void setRGB(int key, float r, float g, float b) {
	setColor(key, pixels.Color(int(r * 255.0f), int(g * 255.0f), int(b * 255.0f)));
}

void setHSV(int key, float h, float s, float v) {
	setColor(key, pixels.ColorHSV(int(h * 65535.0f), int(s * 255.0f), int(v * 255.0f)));
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
	if(!keys[key].down)
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

    Serial.begin(115200);

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

    if(checkMenuStreakPress()) {
        Serial.println("Menu reset detected");
        resetBootloader();
    }

    if(hold(0, 1000)) {
        shutdownLoop();
    }
}


void setupAudio(std::function<void(int16_t*, int)> callback) {
	audioCallback = callback;
	setupI2S();
}
