# Keysmet ONE

## Setup (Arduino IDE)

Install the [Arduino IDE](https://www.arduino.cc/en/software) if you don't
have it, then:

**1. Add two board URLs.** Open *File → Preferences*, find *Additional boards
manager URLs*, and paste both of these (the box takes one URL per line):

```
https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
https://raw.githubusercontent.com/keysmet/ksm-arduino/main/package_keysmet_index.json
```

Click OK.

**2. Install two boards.** Open *Tools → Board → Boards Manager*, then search
for and install each of these:

- **Adafruit nRF52** — the toolchain Keysmet builds on.
- **Keysmet nRF52 Boards**

**3. Install NeoPixel.** Open *Tools → Manage Libraries*, search for
**Adafruit NeoPixel**, and install it. Keysmet uses it to drive the key LEDs.

**4. Pick the board.** *Tools → Board → Keysmet nRF52 Boards → Keysmet ONE
(nRF52840)*.

---

## Run your first example

Reset the device to Bootloader mode, either by double-tapping the reset button or pressing the reset combo twice.

Plug the board in, then open *File → Examples → Keysmet → Hello* and press the
**Upload** arrow.

---

## Writing your own sketch

Start from **Hello** rather than a blank file — *File → Save As* gives you
your own copy to edit.

Every sketch needs these two things:

```cpp
#include <keysmet.h>

void setup() {
    ksm::init();
}

void loop() {
    ksm::loop();
}
```

---

## Advanced: PlatformIO

If you already use [PlatformIO](https://platformio.org/), the repository
builds the same examples:

```bash
git clone https://github.com/keysmet/ksm-arduino
cd ksm-arduino
pio run -t upload
```

Pick which example builds by editing [src/main.cpp](src/main.cpp) — it's a
list of `#include` lines with exactly one uncommented:

```cpp
// #include "../examples/Sine/Sine.ino"
#include "../examples/Bytebeat/Bytebeat.ino"   // ← builds this one
// #include "../examples/Colors/Colors.ino"
```

Only one at a time, since each sketch defines its own `setup()` and `loop()`.
