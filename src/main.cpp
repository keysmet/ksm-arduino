#include <Arduino.h>
#include "keysmet.h"
#include <LSM6DS3.h>


#include <Adafruit_TinyUSB.h>
uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD()
};
Adafruit_USBD_HID usbHID(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);


#include <bluefruit.h>
BLEDis bledis; 	
BLEHidAdafruit bleHID;

namespace ble {

	const char * DEVICE_NAME = "KSM1";

	BLEService dataService("8cee49d4-f8cd-44b5-9986-9860eae25def");
	BLECharacteristic protoCharacIn("f65a39c4-bf69-43e9-abf1-f23a4e0ec8d0");

	void bleCallback(ble_evt_t* evt) {
	}

	void dataWriteCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
		//ksm::logf("dataWriteCallback %d", (int)len);
		// ksm::stream::read((char*)data, len);
	}

	void init() {
		Bluefruit.begin();
		Bluefruit.setTxPower(8);    // Check bluefruit.h for supported values

		// Configure and Start Device Information Service
		bledis.setManufacturer("Keysmet");
		bledis.setModel(DEVICE_NAME);
		bledis.begin();

		/* Start BLE HID
		* Note: Apple requires BLE device must have min connection interval >= 20m
		* ( The smaller the connection interval the faster we could send data).
		* However for HID and MIDI device, Apple could accept min connection interval
		* up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min and max
		* connection interval to 11.25  ms and 15 ms respectively for best performance.
		*/
		bleHID.begin();

		// Set callback for set LED from central
		// bleHID.setKeyboardLedCallback(set_keyboard_led);

		/* Set connection interval (min, max) to your perferred value.
		* Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
		* min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
		*/
		Bluefruit.Periph.setConnInterval(3, 9);
		dataService.begin();
		protoCharacIn.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
		protoCharacIn.setPermission(SECMODE_OPEN, SECMODE_OPEN);
		protoCharacIn.setMaxLen(80);
		protoCharacIn.setWriteCallback(dataWriteCallback);
		protoCharacIn.begin();

		Bluefruit.setEventCallback(bleCallback);

		Bluefruit.setName(DEVICE_NAME);
		Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
		Bluefruit.Advertising.addTxPower();
		Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);

		// Include BLE HID service
		Bluefruit.Advertising.addService(bleHID);
		Bluefruit.Advertising.addService(dataService);

		// There is enough room for the dev name in the advertising packet
		Bluefruit.Advertising.addName();

		/* Start Advertising
		* - Enable auto advertising if disconnected
		* - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
		* - Timeout for fast mode is 30 seconds
		* - Start(timeout) with timeout = 0 will advertise forever (until connected)
		*
		* For recommended advertising interval
		* https://developer.apple.com/library/content/qa/qa1931/_index.html
		*/
		Bluefruit.Advertising.restartOnDisconnect(true);
		Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
		Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
		//Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
	}

	bool isAdvertising = false;
	void advertise(bool enable) {
		if(isAdvertising == enable)
			return;
		if(enable)
			 Bluefruit.Advertising.start(0);
		else
			Bluefruit.Advertising.stop();
		isAdvertising = enable;
		// ksm::logf("BLE advertising: %d", (int)enable);
	}

	bool isConnected() {
		return Bluefruit.connected() > 0;
	}

} // ble

const float FREQUENCY = 440.0f;
const int PHASE_SIZE = int(SAMPLE_RATE / FREQUENCY + 0.5f);
int16_t phaseTable[PHASE_SIZE];

void initPhaseTable() {
    for(int i = 0; i < PHASE_SIZE; ++i) {
        float phase = float(i) / float(PHASE_SIZE);
        float v = sinf(phase * 2.0f * M_PI);
        phaseTable[i] = int16_t(v * 32000.0f);
    }
}

float phaseIndex = 0.0f;
const float phaseIncrement = float(PHASE_SIZE) * FREQUENCY / SAMPLE_RATE;

void audioLoop(int16_t* ptr, int count) {
    for (int i = 0; i < count; ++i) {
        int idx = int(phaseIndex);
        float frac = phaseIndex - idx;
        int nextIdx = (idx + 1) % PHASE_SIZE;
        
        int16_t sample1 = phaseTable[idx];
        int16_t sample2 = phaseTable[nextIdx];
        int16_t iv = int16_t(sample1 + frac * (sample2 - sample1));
        
        *ptr++ = iv;
        *ptr++ = iv;
        
        phaseIndex += phaseIncrement;
        while(phaseIndex >= PHASE_SIZE) {
            phaseIndex -= PHASE_SIZE;
        }
    }
}

void setup() {
	ksm_init();
	initPhaseTable();
	setupAudio(audioLoop);
	IMU.begin();
	usbHID.begin();
	ble::init();
	ble::advertise(true);
}


int keyDown = 0;
void loop() {
	ksm_loop();

	// Wait for USB to be ready before sending reports
	// if (!TinyUSBDevice.mounted()) {
	// 	return;
	// }

	int startKey = 0x1E;
	int newKeyDown = 0;
	for(int i=1; i<=10; ++i) {
		setColor(i, 0x00ff00);
		delay(500);
		ksm_loop();  // TODO
		setColor(i, 0x0);
		// setColor(i, 0x0);
		// if(down(i)) {
		// 	// newKeyDown = startKey + i - 1;
		// 	// usbHID.keyboardPress(0, char('a' + i - 1));
		// 	setColor(i, 0x20FF00);
		// } else {
		// 	// usbHID.keyboardRelease(0);
		// 	setColor(i, 0x0);
		// }
	}

	// if(newKeyDown != keyDown) {
	// 	keyDown = newKeyDown;
	// 	uint8_t codes[6] = { (uint8_t)keyDown, 0, 0, 0, 0, 0 };
	// 	usbHID.keyboardReport(0, 0, codes);
	// 	bleHID.keyboardReport(0, codes);
	// }
}
