/*
  Hello — the smallest possible USB serial test.

  Prints 1 to 10 over USB serial, once per second, then repeats. Open the
  serial monitor at 115200 baud.

  This exists to answer one question on its own: does the board come up on
  USB at all? It touches nothing but ksm::init() and Serial, so if the
  port shows up here the USB stack is fine and any problem is in the
  sketch under test.

  Board: Keysmet ONE (KSM1)
*/

#include <Arduino.h>
#include <keysmet.h>

void setup() {
  // ksm::init() already calls Serial.begin(115200).
  ksm::init();
}

void loop() {
  ksm::loop();

  for (int i = 1; i <= 10; ++i) {
    if(ksm::press(i)) {
      ksm::setColor(i, 0x00ff00);
      Serial.println(i);
    }
    else if(ksm::release(i)) {
      ksm::setColor(i, 0x000000);
    }
  }
}
