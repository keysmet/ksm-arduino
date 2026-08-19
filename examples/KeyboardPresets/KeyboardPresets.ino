/*
  KeyboardPresets — a USB keyboard with several swappable layouts.

  Plug the board in over USB and it enumerates as a plain HID keyboard,
  no pairing needed. The 10 keys don't have one fixed mapping: they
  carry a *preset*, and a short press of MENU (key 0) cycles to the
  next one. Holding MENU for a second still powers the board down, the
  way the library always handles it.

  Each preset has its own hue, so you can tell at a glance which one is
  active. Keys are dimly lit at rest and go bright when pressed; a key
  with nothing mapped to it (a dead key) stays dark and sends nothing.
  On every preset change the board sweeps a bright column across in the
  new hue, so the switch is obvious even if you weren't looking.

  Presets (2 rows of 5, "." is a dead key):

    0 wasd  green            1 arrows  azure          2 numbers  amber
      .  W  .  .   SPC         .  UP .  BSP ENT         1 2 3 4 5
      A  S  D  .   SHF         LT DN RT SPC ALT         6 7 8 9 0

    3 function  violet       4 navigate  magenta      5 edit  red
      F1 F2 F3 F4  F5          HOME PGUP END  .   .     ^Z ^X ^C ^V .
      F6 F7 F8 F9  F10         PGDN DEL  ^TAB ^W  ESC   ^Y ^A ^F ^S ESC

    6 numpad  cyan
      7 8 9 +  .
      4 5 6 0  ENT

  Combos (a modifier plus a key, like Ctrl+C) are supported: the
  modifier is pressed just before the key and released just after, and
  the key washes out toward white while held so combos look distinct.

  How it works: ksm::initKeyboardUSB() sets the board up as a USB HID
  keyboard, and ksm::setKeyboard() tracks which keys are held and ships
  the report. The library owns the transport and the key state; this
  sketch only decides which physical key means what. Swap in
  ksm::initKeyboardBLE() for a wireless keyboard, or call both.

  Note: media/consumer keys aren't sent here — the library's
  setKeyboard() ignores CONSUMER_* codes, so a media preset would look
  alive but send nothing.

  Board: Keysmet ONE (KSM1)
*/

#include <keysmet.h>
#include <math.h>

constexpr int KEY_LO = 1;
constexpr int KEY_HI = 10;
constexpr int COLS = 5;
constexpr int KEY_COUNT_GRID = KEY_HI - KEY_LO + 1; // 10

namespace kc = ksm::keycodes;

// One physical key's mapping. `mod` is a modifier keycode (CONTROL_LEFT,
// SHIFT_LEFT, ...) held while `code` is sent, or NONE for a plain key.
// A `code` of NONE makes the key dead: dark, and it sends nothing.
struct Binding {
  uint8_t code;
  uint8_t mod;
};

constexpr Binding key(uint8_t code) { return Binding{ code, kc::NONE }; }
constexpr Binding combo(uint8_t mod, uint8_t code) { return Binding{ code, mod }; }
constexpr Binding dead() { return Binding{ kc::NONE, kc::NONE }; }

struct Preset {
  const char *name;
  float hue;              // 0..1, the preset's identity color
  Binding keys[KEY_COUNT_GRID];
};

// Order matters: MENU walks this list top to bottom, then wraps.
const Preset PRESETS[] = {
  // WASD movement, the way a game expects it: W above S, A/D either
  // side, plus space to jump and shift to sprint.
  { "wasd", 0.33f, {           // green
      dead(),      key(kc::W), dead(),      dead(), key(kc::SPACE),
      key(kc::A),  key(kc::S), key(kc::D),  dead(), key(kc::SHIFT_LEFT),
  }},

  // Arrow cluster with the usual text-editing neighbours.
  { "arrows", 0.55f, {         // azure
      dead(),              key(kc::ARROW_UP),   dead(),               key(kc::BACKSPACE), key(kc::ENTER),
      key(kc::ARROW_LEFT), key(kc::ARROW_DOWN), key(kc::ARROW_RIGHT), key(kc::SPACE),     key(kc::ALT_LEFT),
  }},

  // Digits 1-9 then 0, in reading order.
  { "numbers", 0.08f, {        // amber
      key(kc::NUM_1), key(kc::NUM_2), key(kc::NUM_3), key(kc::NUM_4), key(kc::NUM_5),
      key(kc::NUM_6), key(kc::NUM_7), key(kc::NUM_8), key(kc::NUM_9), key(kc::NUM_0),
  }},

  // F1-F10.
  { "function", 0.75f, {       // violet
      key(kc::F1), key(kc::F2), key(kc::F3), key(kc::F4), key(kc::F5),
      key(kc::F6), key(kc::F7), key(kc::F8), key(kc::F9), key(kc::F10),
  }},

  // Page/document movement plus tab switching. Deliberately sparse —
  // the unused slots stay dark, which is what dead keys are for.
  { "navigate", 0.86f, {       // magenta
      key(kc::HOME),      key(kc::PAGE_UP), key(kc::END),                     dead(),                         dead(),
      key(kc::PAGE_DOWN), key(kc::DELETE),  combo(kc::CONTROL_LEFT, kc::TAB), combo(kc::CONTROL_LEFT, kc::W), key(kc::ESCAPE),
  }},

  // Editing combos: everything here is Ctrl+something except ESC.
  { "edit", 0.02f, {           // red
      combo(kc::CONTROL_LEFT, kc::Z), combo(kc::CONTROL_LEFT, kc::X), combo(kc::CONTROL_LEFT, kc::C), combo(kc::CONTROL_LEFT, kc::V), dead(),
      combo(kc::CONTROL_LEFT, kc::Y), combo(kc::CONTROL_LEFT, kc::A), combo(kc::CONTROL_LEFT, kc::F), combo(kc::CONTROL_LEFT, kc::S), key(kc::ESCAPE),
  }},

  // A numeric keypad squeezed into two rows: top digits plus add,
  // bottom digits plus zero and enter.
  { "numpad", 0.50f, {         // cyan
      key(kc::KEYPAD_7), key(kc::KEYPAD_8), key(kc::KEYPAD_9), key(kc::KEYPAD_ADD), dead(),
      key(kc::KEYPAD_4), key(kc::KEYPAD_5), key(kc::KEYPAD_6), key(kc::KEYPAD_0),   key(kc::KEYPAD_ENTER),
  }},
};

const int PRESET_COUNT = sizeof(PRESETS) / sizeof(PRESETS[0]);

int gPreset = 0;
double gSwitchTime = 0;   // when the current preset was selected

inline const Preset &current() { return PRESETS[gPreset]; }
inline const Binding &bindingFor(int k) { return current().keys[k - KEY_LO]; }
inline bool isDead(const Binding &b) { return b.code == kc::NONE; }
inline int colOf(int k) { return (k - KEY_LO) % COLS; }

// Release everything the host thinks is held, so a key still down when
// the mapping changes can't stick as a phantom keypress.
void selectPreset(int index) {
  ksm::clearKeyboard();
  gPreset = index;
  gSwitchTime = ksm::getTime();
}

void setup() {
  ksm::init();
  ksm::initKeyboardUSB();

  selectPreset(0);
}

void loop() {
  ksm::loop();

  // A short MENU press swaps layouts. Holding MENU for a second is the
  // library's power-off gesture, so don't act until MENU comes back up
  // without having crossed that threshold.
  static bool menuHeldLong = false;
  if (ksm::hold(KEY_MENU, 1000))
    menuHeldLong = true;
  if (ksm::release(KEY_MENU)) {
    if (!menuHeldLong)
      selectPreset((gPreset + 1) % PRESET_COUNT);
    menuHeldLong = false;
  }

  bool connected = ksm::keyboardConnected();

  if (connected) {
    for (int k = KEY_LO; k <= KEY_HI; ++k) {
      const Binding &b = bindingFor(k);
      if (isDead(b))
        continue;

      // Modifier first on the way down, last on the way up, so the host
      // never sees the bare key without its modifier.
      if (ksm::press(k)) {
        if (b.mod != kc::NONE)
          ksm::setKeyboard(b.mod, true);
        ksm::setKeyboard(b.code, true);
      }
      if (ksm::release(k)) {
        ksm::setKeyboard(b.code, false);
        if (b.mod != kc::NONE)
          ksm::setKeyboard(b.mod, false);
      }
    }
  }

  // --- lighting ---------------------------------------------------
  float t = (float)ksm::getTime();

  if (!connected) {
    // Slow blue pulse until the host enumerates us — no layout yet.
    float pulse = 0.5f + 0.5f * sinf(t * 10.0f);
    for (int k = KEY_LO; k <= KEY_HI; ++k)
      ksm::setHSV(k, 0.62f, 1.0f, 0.05f + pulse * 0.45f);
    return;
  }

  const Preset &p = current();

  // Just after a switch, sweep a bright column left-to-right in the new
  // preset's hue. It doubles as a "this is the color now" announcement.
  const float SWEEP_SECONDS = 0.45f;
  float sinceSwitch = (float)(ksm::getTime() - gSwitchTime);
  float sweepCol = (sinceSwitch / SWEEP_SECONDS) * COLS;
  bool sweeping = sinceSwitch < SWEEP_SECONDS;

  for (int k = KEY_LO; k <= KEY_HI; ++k) {
    const Binding &b = bindingFor(k);

    // Dead keys stay dark so the usable layout reads at a glance.
    float v = isDead(b) ? 0.0f : 0.10f;
    if (!isDead(b) && ksm::down(k))
      v = 1.0f;

    if (sweeping) {
      float d = fabsf(sweepCol - colOf(k));
      if (d < 1.0f)
        v = fmaxf(v, 1.0f - d);
    }

    // Held combos wash toward white, so they read differently from a
    // plain key at the same brightness.
    float sat = (b.mod != kc::NONE && ksm::down(k)) ? 0.35f : 1.0f;
    ksm::setHSV(k, p.hue, sat, v);
  }
}
