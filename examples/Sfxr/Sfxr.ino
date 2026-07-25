/*
  Sfxr — retro game sound effects.

  sfxr is the classic 8-bit sound effect synthesizer: a handful of
  parameters (waveform, envelope, frequency slide, vibrato, filters)
  describe a whole sound. See https://www.drpetter.se/project_sfxr.html

  Each of the 10 keys triggers a different effect. Parameters below are
  plain structs, so you can tweak numbers and hear the result immediately —
  that is the whole point of sfxr.

  Field order matches sfxr_params in sfxr.h.
  wave_type: 0 = square, 1 = sawtooth, 2 = sine, 3 = noise.

  Board: Keysmet ONE (KSM1)
*/

#include <keysmet.h>
#include <sfxr.h>

// "pickup/coin": bright square blip with an upward arpeggio flick.
static const sfxr_params sfxCoin = {
    /* wave_type       */ 0,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.08f,
    /* p_env_punch     */ 0.45f,
    /* p_env_decay     */ 0.40f,
    /* p_base_freq     */ 0.55f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ 0.0f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.0f,
    /* p_vib_speed     */ 0.0f,
    /* p_arp_mod       */ 0.55f,
    /* p_arp_speed     */ 0.55f,
    /* p_duty          */ 0.35f,
    /* p_duty_ramp     */ 0.0f,
    /* p_repeat_speed  */ 0.0f,
    /* p_pha_offset    */ 0.0f,
    /* p_pha_ramp      */ 0.0f,
    /* p_lpf_freq      */ 1.0f,
    /* p_lpf_ramp      */ 0.0f,
    /* p_lpf_resonance */ 0.0f,
    /* p_hpf_freq      */ 0.0f,
    /* p_hpf_ramp      */ 0.0f,
    /* sound_vol       */ 1.0f,
};

// "jump": short square tone ramping up in pitch.
static const sfxr_params sfxJump = {
    0, 0.0f, 0.15f, 0.35f, 0.28f, 0.35f, 0.0f, 0.28f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

// "hurt": noisy downward-sliding hit.
static const sfxr_params sfxHurt = {
    3, 0.0f, 0.10f, 0.30f, 0.25f, 0.35f, 0.0f, -0.35f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.10f, 0.0f, 1.0f,
};

// "laser/shoot": sawtooth zap sliding down.
static const sfxr_params sfxShoot = {
    1, 0.0f, 0.12f, 0.40f, 0.20f, 0.50f, 0.0f, -0.30f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

// "blip": short sawtooth click with a small arpeggio.
static const sfxr_params sfxBlip = {
    1, 0.0f, 0.083f, 0.322f, 0.310f, 0.449f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.286f, 0.579f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    1.0f,
};

// "select": soft sine chirp sliding up — UI confirm.
static const sfxr_params sfxSelect = {
    2, 0.0f, 0.06f, 0.10f, 0.12f, 0.45f, 0.0f, 0.15f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

// "error": harsh sawtooth stepping down — negative feedback.
static const sfxr_params sfxError = {
    1, 0.0f, 0.20f, 0.10f, 0.30f, 0.28f, 0.0f, -0.10f, 0.0f, 0.0f, 0.0f,
    -0.25f, 0.65f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    1.0f,
};

// "alarm": sustained square that stutters via repeat.
static const sfxr_params sfxAlarm = {
    0, 0.0f, 0.60f, 0.0f, 0.15f, 0.40f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.50f, 0.0f, 0.55f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

// "thruster": low filtered noise rumble with slow vibrato.
static const sfxr_params sfxThruster = {
    3, 0.10f, 0.50f, 0.0f, 0.25f, 0.18f, 0.0f, 0.0f, 0.0f, 0.15f, 0.40f, 0.0f,
    0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 0.45f, 0.0f, 0.20f, 0.0f, 0.0f, 1.0f,
};

// "powerup": rising sine sweep with vibrato.
static const sfxr_params sfxPowerup = {
    2, 0.0f, 0.30f, 0.0f, 0.40f, 0.25f, 0.0f, 0.22f, 0.0f, 0.35f, 0.45f, 0.0f,
    0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

// Key N triggers sfxTable[N-1].
static const sfxr_params *const sfxTable[10] = {
    &sfxCoin,  &sfxJump,  &sfxHurt,     &sfxShoot,   &sfxBlip,
    &sfxSelect, &sfxError, &sfxAlarm, &sfxThruster, &sfxPowerup,
};

// Color per effect, so each key looks like what it sounds like.
static const int sfxColor[10] = {
    0xFFC000, 0x00FF40, 0xFF0030, 0x00E0FF, 0xFFFFFF,
    0x40FF80, 0xFF2000, 0xFF8000, 0x4040FF, 0xFF00FF,
};

static sfxr_state sfx;

// Index the main loop wants the audio thread to (re)start, or -1 for none.
static volatile int trigger = -1;

void audioCallback(int16_t *buffer, int count) {
  // Pick up a pending trigger and restart the synth from the top.
  int t = trigger;
  if (t >= 0 && t < 10) {
    trigger = -1;
    sfxr_reset(&sfx, sfxTable[t]);
  }

  if (sfxr_playing(&sfx)) {
    // Fills both channels and pads with silence when the sound ends.
    sfxr_generate_s16_stereo(&sfx, buffer, count);
  } else {
    for (int i = 0; i < count * 2; ++i)
      buffer[i] = 0;
  }
}

void setup() {
  ksm::init();
  ksm::setupAudio(audioCallback);
}

void loop() {
  ksm::loop();

  for (int key = 1; key <= 10; ++key) {
    if (ksm::press(key))
      trigger = key - 1;
  }

  // Each key wears its effect's color, brightened while held.
  for (int key = 1; key <= 10; ++key) {
    int color = sfxColor[key - 1];
    if (!ksm::down(key)) {
      // Dim to roughly 1/8 when idle.
      int r = ((color >> 16) & 0xFF) >> 3;
      int g = ((color >> 8) & 0xFF) >> 3;
      int b = (color & 0xFF) >> 3;
      color = (r << 16) | (g << 8) | b;
    }
    ksm::setColor(key, color);
  }
}
