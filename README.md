# Keysmet ONE

Firmware SDK and examples for the Keysmet ONE (KSM1) — a 10-key programmable
keyboard built on an nRF52840.

The board can be programmed two ways. **Arduino IDE** is the easy path and the
one to point users at. **PlatformIO** is what this repository is set up for
day to day, and it builds the exact same example sources.

---

## Arduino IDE

### Install

1. Install the **Adafruit nRF52 BSP**, which provides the compiler, the nRF5
   core and the upload tool that this board builds on top of. In
   *File → Preferences → Additional boards manager URLs*, add:

   ```
   https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
   ```

   Then in *Tools → Board → Boards Manager*, search for **Adafruit nRF52** and
   install it.

2. Add the Keysmet index to the same preferences field (the box takes several
   URLs, one per line):

   ```
   https://raw.githubusercontent.com/keysmet/ksm-arduino/main/package_keysmet_index.json
   ```

3. In Boards Manager, search for **Keysmet** and install *Keysmet nRF52
   Boards*.

4. Select *Tools → Board → Keysmet nRF52 Boards → Keysmet ONE (nRF52840)*.

Step 1 is not optional — the Keysmet platform deliberately reuses Adafruit's
core and toolchain rather than shipping its own copy, so installing Keysmet
alone will fail to compile.

### Run an example

The library and its examples are bundled inside the board package, so there is
nothing else to install. With the board selected, open
*File → Examples → Keysmet* and pick one:

| Example | What it does |
| --- | --- |
| `Sine` | Audio hello-world — one note per key, from a wavetable |
| `Bytebeat` | One-line algorithmic music; each key selects a formula |
| `Sfxr` | Retro game sound effects, ten presets |
| `Colors` | LED gradients, patterns and animations |
| `BleKeyboard` | Bluetooth HID keyboard |
| `KeyboardPresets` | USB HID keyboard; MENU cycles through layouts |

Press *Upload*. The board resets into its bootloader on its own via the
1200-baud touch, so you should not need to press anything.

If the upload cannot find the port, put the board into bootloader mode by
double-tapping the reset button — the LEDs will breathe to confirm — and pick
the new port that appears under *Tools → Port*.

---

## PlatformIO

```bash
git clone https://github.com/keysmet/ksm-arduino
cd ksm-arduino
pio run -t upload
```

Everything is preconfigured in [platformio.ini](platformio.ini): the board
definition comes from [boards/ksm1.json](boards/ksm1.json), and dependencies
are pulled automatically on first build.

### Choosing an example

PlatformIO has no equivalent of the Arduino IDE's sketch selector, so
[src/main.cpp](src/main.cpp) does the job: it is a list of `#include` lines,
one per example, with exactly one uncommented.

```cpp
// #include "../examples/Sine/Sine.ino"
#include "../examples/Bytebeat/Bytebeat.ino"   // ← builds this one
// #include "../examples/Colors/Colors.ino"
```

Uncomment the one you want, comment out the previous one, and `pio run -t
upload`. Only one may be active at a time, since each sketch defines its own
`setup()` and `loop()`.

The `.ino` files under [examples/](examples/) are the single source of truth.
Arduino opens them directly and PlatformIO compiles them through the include
above, so there is no conversion step and no generated copies to fall out of
sync.

One consequence worth knowing: PlatformIO's dependency finder scans
`main.cpp`, not the `.ino` it pulls in. Libraries used only inside a sketch
still have to be named by an `#include` at the top of `main.cpp` — that is why
`bluefruit.h`, `keysmet.h` and `sfxr.h` appear there even though `main.cpp`
itself uses none of them.

### Uploading over a debugger

The default is DFU over USB. To use a J-Link instead, swap the two lines near
the top of `platformio.ini`:

```ini
; upload_protocol = nrfutil
upload_protocol = jlink
```

---

## Repository layout

```
examples/            The example sketches — source of truth for both toolchains
lib/keysmet/         The Keysmet library (ksm:: API, sfxr synthesis)
variants/ksm1/       Pin mapping and board init — shared by both toolchains
boards/ksm1.json     PlatformIO board definition
arduino/keysmet/     Arduino platform metadata (boards.txt, platform.txt)
tools/               Packaging and flashing helpers
```

`variants/` is shared rather than duplicated. PlatformIO reads it directly via
`board_build.variants_dir`, and the packaging script copies it into the
Arduino archive at build time. It was duplicated once and the two copies
silently drifted apart on three pin assignments, which is why it is now
generated rather than checked in twice.

---

## Cutting a release

Releases are what make the Arduino install work: Boards Manager reads the
index from the `main` branch and downloads the archive it points at.

```bash
bash tools/build-arduino-package.sh 1.0.0
```

This bundles the platform, the variant, and the Keysmet library with its
examples into `keysmet-nrf52-<version>.tar.bz2`, then rewrites the checksum,
size and filename in
[package_keysmet_index.json](package_keysmet_index.json).

Then commit and publish:

```bash
git commit -am "Release v1.0.0"
git push
gh release create v1.0.0 keysmet-nrf52-1.0.0.tar.bz2 \
  --title "v1.0.0" --notes "Keysmet nRF52 board support"
```

Push the index **before** creating the release. Boards Manager fetches the
index from `raw.githubusercontent.com` on the `main` branch, so an index that
is still sitting in your working tree describes an archive nobody can see.

For a new version, bump `version` in both
[arduino/keysmet/platform.txt](arduino/keysmet/platform.txt) and
[lib/keysmet/library.properties](lib/keysmet/library.properties), then add a
matching entry to the `platforms` array in the index — the script updates an
existing entry but will not invent a new one, and warns if it finds no match.

The tarball itself is not committed; it is a build artifact and is gitignored.
