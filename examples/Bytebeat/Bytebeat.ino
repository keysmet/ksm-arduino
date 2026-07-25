/*
  Bytebeat — one-line algorithmic music.

  A bytebeat is a single expression over a counter `t` that evaluates to a
  byte. Feed those bytes to a speaker and you get surprisingly musical
  results out of a few operators. See http://canonical.org/~kragen/bytebeat/

  Press keys 1..10 to switch between ten different tunes. Key 1 restarts
  the currently playing tune from t = 0.

  Bytebeats are written for an 8kHz sample rate, so we tick `t` at 8kHz
  using an accumulator regardless of the board's real rate.

  Board: Keysmet ONE (KSM1)
*/

#include <keysmet.h>

static const int TUNE_COUNT = 10;

// Selected tune, written by loop() and read by the audio callback.
static volatile int tune = 0;
// Set to true by loop() to ask the audio callback to restart at t = 0.
static volatile bool restart = false;

// Audio-thread state.
static uint32_t t = 0;
static uint32_t rateAcc = 0;

// Each case is one classic bytebeat expression. `t` advances at 8kHz.
static uint8_t bytebeat(int which, uint32_t t) {
  switch (which) {
  case 0:
    // "the 42 melody" — the original bytebeat by viznut.
    return t * (((t >> 12) | (t >> 8)) & (63 & (t >> 4)));
  case 1:
    return t * ((t & 4096 ? (t % 65536 < 59392 ? 7 : t >> 6) : 16) + (1 & t >> 14)) >>
               (3 & -t >> (t & 2048 ? 2 : 10)) |
           t >> (t & 16384 ? (t & 4096 ? 4 : 3) : 2);
  case 2:
    return t * (2 & t >> 13 ? 7 : 5) * (3 - (3 & t >> 9) + (3 & t >> 8)) >>
               (3 & -t >> ((t & 4096 | (t >> 11) % 32 > 28) ? 2 : 16)) |
           t >> 3;
  case 3:
    return (~t >> 2) * ((127 & t * (7 & t >> 10)) < (245 & t * (2 + (5 & t >> 14))));
  case 4:
    return t * (t >> (t & 4096 ? t * t >> 12 : t >> 12)) | t << (t >> 8) | t >> 4;
  case 5:
    return t >> 5 | t >> 4 | ((t % 42 * (t >> 4) | 357052691 - (t >> 4)) / (t >> 16)) ^ (t | t >> 4);
  case 6:
    return t / (((t >> (3 - (t >> 14) % 2)) % (26 >> (t >> 16) % 3))) % 1024 / 12 << 9 / (t >> 5 & 127);
  case 7:
    return (t >> 10 ^ t >> 11) % 5 * ((t >> 14 & 3 ^ t >> 15 & 1) + 1) * t % 99 +
           ((3 + (t >> 14 & 3) - (t >> 16 & 1)) / 3 * t % 99 & 64);
  case 8:
    return ((t * (t >> 12)) << (-t >> 10 & 7)) & -t >> 2;
  case 9:
    return (t % 125 & t >> 8) | t >> 4 | t * t >> 8 & t >> 8;
  default:
    return 128; // silence (midpoint)
  }
}

void audioCallback(int16_t *buffer, int count) {
  if (restart) {
    restart = false;
    t = 0;
    rateAcc = 0;
  }

  int which = tune;

  for (int i = 0; i < count; ++i) {
    // Advance t at 8kHz no matter what the hardware rate is.
    rateAcc += 8000;
    if (rateAcc >= uint32_t(KSM_SAMPLE_RATE)) {
      rateAcc -= uint32_t(KSM_SAMPLE_RATE);
      t++;
    }

    // Bytebeats produce an unsigned byte centred on 128. Shift to signed
    // and scale up to 16-bit, at reduced volume — these are harsh.
    uint8_t s = bytebeat(which, t);
    int16_t sample = (int16_t(s) - 128) << 6;

    *buffer++ = sample; // left
    *buffer++ = sample; // right
  }
}

void setup() {
  ksm::init();
  ksm::setupAudio(audioCallback);
}

void loop() {
  ksm::loop();

  for (int key = 1; key <= TUNE_COUNT; ++key) {
    if (ksm::press(key)) {
      int selected = key - 1;
      // Pressing the key of the tune already playing restarts it.
      if (selected == tune)
        restart = true;
      tune = selected;
    }
  }

  // Highlight the active tune; the rest glow faintly.
  for (int key = 1; key <= 10; ++key) {
    ksm::setColor(key, (key - 1) == tune ? 0xFF4000 : 0x040404);
  }
}
