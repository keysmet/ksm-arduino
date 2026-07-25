/*
  Sine — Keysmet audio "hello world".

  Plays a sine wave through the speaker. Each of the 10 keys plays a
  different note of a C major scale, and lights up while held.

  The audio callback runs on the audio interrupt and must be fast: no
  Serial, no delay(), no allocation. It fills a stereo buffer of `count`
  frames (left and right interleaved).

  Board: Keysmet ONE (KSM1)
*/

#include <keysmet.h>
#include <math.h>

// One sine cycle, precomputed. Reading a table is far cheaper than calling
// sinf() for every sample at 22kHz.
static const int TABLE_SIZE = 1024;
static int16_t sineTable[TABLE_SIZE];

// C major scale, one note per key (keys are 1..10).
static const float NOTE_FREQ[10] = {
    261.63f, // C4  key 1
    293.66f, // D4  key 2
    329.63f, // E4  key 3
    349.23f, // F4  key 4
    392.00f, // G4  key 5
    440.00f, // A4  key 6
    493.88f, // B4  key 7
    523.25f, // C5  key 8
    587.33f, // D5  key 9
    659.25f, // E5  key 10
};

// Shared with the audio callback. `volatile` because loop() writes them and
// the audio interrupt reads them.
static volatile float targetFreq = 0.0f; // 0 = silent
static volatile float targetAmp = 0.0f;  // 0..1

// Audio-thread state — only touched inside audioCallback().
static float phase = 0.0f; // in table units, 0..TABLE_SIZE
static float amp = 0.0f;   // smoothed amplitude, avoids clicks

void audioCallback(int16_t *buffer, int count) {
  float freq = targetFreq;
  float target = targetAmp;

  // Table steps per sample for this frequency.
  float phaseInc = freq * float(TABLE_SIZE) / KSM_SAMPLE_RATE;

  for (int i = 0; i < count; ++i) {
    // Glide the amplitude toward its target so notes fade in/out instead of
    // clicking. ~1ms time constant.
    amp += (target - amp) * 0.002f;

    phase += phaseInc;
    if (phase >= TABLE_SIZE)
      phase -= TABLE_SIZE;

    int16_t sample = int16_t(sineTable[int(phase)] * amp);

    *buffer++ = sample; // left
    *buffer++ = sample; // right
  }
}

void setup() {
  for (int i = 0; i < TABLE_SIZE; ++i) {
    float t = float(i) / float(TABLE_SIZE);
    sineTable[i] = int16_t(sinf(t * 2.0f * PI) * 32000.0f);
  }

  ksm::init();
  ksm::setupAudio(audioCallback);
}

void loop() {
  ksm::loop();

  // Last key pressed wins; releasing it silences the note.
  int held = 0;
  for (int key = 1; key <= 10; ++key) {
    if (ksm::down(key))
      held = key;
  }

  if (held) {
    targetFreq = NOTE_FREQ[held - 1];
    targetAmp = 0.6f;
  } else {
    targetAmp = 0.0f;
  }

  // Light the held key, dim the rest.
  for (int key = 1; key <= 10; ++key) {
    ksm::setColor(key, key == held ? 0x00FF80 : 0x000000);
  }
}
