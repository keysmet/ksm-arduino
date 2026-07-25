/*
  BleKeyboard — act as a Bluetooth keyboard.

  Pairs over BLE HID and sends a keystroke when you press a key. Pair it
  from your computer or phone (look for "KSM1" in the Bluetooth settings),
  then open a text editor and press keys.

  How it works: ksm::setKeyboard() tracks which HID keys are held and
  builds a standard 6-key HID report. Whenever that report changes, the
  callback set with ksm::setKeyboardReportCallback() fires and we forward
  it to the BLE HID service. The library handles the key state; this
  sketch only decides which physical key maps to which keycode.

  Default mapping (2 rows of 5):

      TAB   UP    ESC   BKSP  ENTER
      LEFT  DOWN  RIGHT SPACE ALT

  The LED turns blue while advertising and green once connected.

  Board: Keysmet ONE (KSM1)
*/

#include <keysmet.h>
#include <bluefruit.h>

BLEDis bledis;
BLEHidAdafruit bleHID;

static const char *DEVICE_NAME = "KSM1";

// Physical key N (1..10) sends keyMap[N-1].
static const uint8_t keyMap[10] = {
    ksm::keycodes::TAB,        // key 1
    ksm::keycodes::ARROW_UP,   // key 2
    ksm::keycodes::ESCAPE,     // key 3
    ksm::keycodes::BACKSPACE,  // key 4
    ksm::keycodes::ENTER,      // key 5
    ksm::keycodes::ARROW_LEFT, // key 6
    ksm::keycodes::ARROW_DOWN, // key 7
    ksm::keycodes::ARROW_RIGHT,// key 8
    ksm::keycodes::SPACE,      // key 9
    ksm::keycodes::ALT_LEFT,   // key 10
};

void startAdvertising() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  Bluefruit.Advertising.addService(bleHID);
  Bluefruit.Advertising.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); // in units of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // seconds in fast mode
  Bluefruit.Advertising.start(0);             // 0 = advertise until connected
}

void setup() {
  ksm::init();

  Bluefruit.begin();
  Bluefruit.setTxPower(8);
  Bluefruit.setName(DEVICE_NAME);

  bledis.setManufacturer("Keysmet");
  bledis.setModel(DEVICE_NAME);
  bledis.begin();

  bleHID.begin();

  // Apple recommends a min connection interval >= 11.25ms for HID.
  // Units of 1.25ms: 3 = 11.25ms, 9 = 15ms.
  Bluefruit.Periph.setConnInterval(3, 9);

  // The library assembles the HID report; we just ship it over BLE.
  ksm::setKeyboardReportCallback([](uint8_t modifiers, uint8_t *keys) {
    bleHID.keyboardReport(modifiers, keys);
  });

  startAdvertising();
}

void loop() {
  ksm::loop();

  bool connected = Bluefruit.connected() > 0;

  // Only send keystrokes once a host is actually connected.
  if (connected) {
    for (int key = 1; key <= 10; ++key) {
      if (ksm::press(key))
        ksm::setKeyboard(keyMap[key - 1], true);
      if (ksm::release(key))
        ksm::setKeyboard(keyMap[key - 1], false);
    }
  }

  // Green when connected, slow blue pulse while waiting to pair.
  for (int key = 1; key <= 10; ++key) {
    if (connected) {
      ksm::setColor(key, ksm::down(key) ? 0x00FF00 : 0x001A00);
    } else {
      float pulse = 0.5f + 0.5f * sinf(float(ksm::getTime()) * 3.0f);
      ksm::setHSV(key, 0.62f, 1.0f, 0.05f + pulse * 0.45f);
    }
  }
}
