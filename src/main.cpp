#include <Arduino.h>
#include "keysmet.h"
#include "colordemo.h"
#include "sfxr.h"
#include <LSM6DS3.h>

#include <MIDI.h>

#include <Adafruit_TinyUSB.h>
uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD()
};
Adafruit_USBD_HID usbHID(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);


// Adafruit_USBD_MIDI usb_midi;
// MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI);
// BLEMIDI_CREATE_DEFAULT_INSTANCE()

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

		// blemidi.begin();

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
		// Bluefruit.Advertising.addService(blemidi);
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

// C major scale over 2 octaves: C5 to C7 (15 notes)
static const float SCALE_FREQS[] = {
    523.25f,  // C5
    587.33f,  // D5
    659.25f,  // E5
    698.46f,  // F5
    783.99f,  // G5
    880.00f,  // A5
    987.77f,  // B5
    1046.50f, // C6
    1174.66f, // D6
    1318.51f, // E6
    1396.91f, // F6
    1567.98f, // G6
    1760.00f, // A6
    1975.53f, // B6
    2093.00f, // C7
};
static const int NOTE_COUNT = sizeof(SCALE_FREQS) / sizeof(SCALE_FREQS[0]);
static const int NOTE_DURATION_SAMPLES = int(KSM_SAMPLE_RATE * 0.3f); // 300ms per note

// Phase table sized for one full sine cycle at high resolution
const int PHASE_SIZE = 1024;
int16_t phaseTable[PHASE_SIZE];

void initPhaseTable() {
    for(int i = 0; i < PHASE_SIZE; ++i) {
        float phase = float(i) / float(PHASE_SIZE);
        float v = sinf(phase * 2.0f * M_PI);
        phaseTable[i] = int16_t(v * 32000.0f);
    }
}

float phaseIndex = 0.0f;
int currentNote = 0;
int noteSampleCount = 0;
int musicSelect = 0;

// SFXR sound effect triggered on button 1 press.
static const sfxr_params sfxBlip = {
    /* wave_type       */ 1,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.08310601907199754f,
    /* p_env_punch     */ 0.32172773912245023f,
    /* p_env_decay     */ 0.30969441920201457f,
    /* p_base_freq     */ 0.4493456434163772f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ 0.0f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.0f,
    /* p_vib_speed     */ 0.0f,
    /* p_arp_mod       */ 0.28557554372743427f,
    /* p_arp_speed     */ 0.5789224258815204f,
    /* p_duty          */ 0.0f,
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

// --- Additional SFXR sound effects -----------------------------------------
// Field order matches sfxr_params (see sfxr.h). wave_type: 0 square,
// 1 sawtooth, 2 sine, 3 noise. Values follow the standard sfxr [-1,1]/[0,1]
// ranges. sound_vol is the master gain (1.0 drives the sustained body to full
// scale; see SFXR_POST_GAIN in sfxr.h).

// "coin"/pickup: bright square blip with an upward arpeggio flick.
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

// "jump": short square tone that ramps up in pitch.
static const sfxr_params sfxJump = {
    /* wave_type       */ 0,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.15f,
    /* p_env_punch     */ 0.35f,
    /* p_env_decay     */ 0.28f,
    /* p_base_freq     */ 0.35f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ 0.28f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.0f,
    /* p_vib_speed     */ 0.0f,
    /* p_arp_mod       */ 0.0f,
    /* p_arp_speed     */ 0.0f,
    /* p_duty          */ 0.50f,
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

// "hurt": noisy downward-sliding hit.
static const sfxr_params sfxHurt = {
    /* wave_type       */ 3,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.10f,
    /* p_env_punch     */ 0.30f,
    /* p_env_decay     */ 0.25f,
    /* p_base_freq     */ 0.35f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ -0.35f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.0f,
    /* p_vib_speed     */ 0.0f,
    /* p_arp_mod       */ 0.0f,
    /* p_arp_speed     */ 0.0f,
    /* p_duty          */ 0.50f,
    /* p_duty_ramp     */ 0.0f,
    /* p_repeat_speed  */ 0.0f,
    /* p_pha_offset    */ 0.0f,
    /* p_pha_ramp      */ 0.0f,
    /* p_lpf_freq      */ 1.0f,
    /* p_lpf_ramp      */ 0.0f,
    /* p_lpf_resonance */ 0.0f,
    /* p_hpf_freq      */ 0.10f,
    /* p_hpf_ramp      */ 0.0f,
    /* sound_vol       */ 1.0f,
};

// "shoot"/laser: sawtooth zap sliding down in pitch.
static const sfxr_params sfxShoot = {
    /* wave_type       */ 1,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.12f,
    /* p_env_punch     */ 0.40f,
    /* p_env_decay     */ 0.20f,
    /* p_base_freq     */ 0.50f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ -0.30f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.0f,
    /* p_vib_speed     */ 0.0f,
    /* p_arp_mod       */ 0.0f,
    /* p_arp_speed     */ 0.0f,
    /* p_duty          */ 0.50f,
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

// "select": short sine chirp sliding up — soft UI confirm.
static const sfxr_params sfxSelect = {
    /* wave_type       */ 2,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.06f,
    /* p_env_punch     */ 0.10f,
    /* p_env_decay     */ 0.12f,
    /* p_base_freq     */ 0.45f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ 0.15f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.0f,
    /* p_vib_speed     */ 0.0f,
    /* p_arp_mod       */ 0.0f,
    /* p_arp_speed     */ 0.0f,
    /* p_duty          */ 0.50f,
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

// "error": harsh sawtooth with a downward arpeggio step — negative feedback.
static const sfxr_params sfxError = {
    /* wave_type       */ 1,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.20f,
    /* p_env_punch     */ 0.10f,
    /* p_env_decay     */ 0.30f,
    /* p_base_freq     */ 0.28f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ -0.10f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.0f,
    /* p_vib_speed     */ 0.0f,
    /* p_arp_mod       */ -0.25f,
    /* p_arp_speed     */ 0.65f,
    /* p_duty          */ 0.50f,
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

// "alarm": sustained square tone that stutters via repeat — warning beeper.
static const sfxr_params sfxAlarm = {
    /* wave_type       */ 0,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.60f,
    /* p_env_punch     */ 0.0f,
    /* p_env_decay     */ 0.15f,
    /* p_base_freq     */ 0.40f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ 0.0f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.0f,
    /* p_vib_speed     */ 0.0f,
    /* p_arp_mod       */ 0.0f,
    /* p_arp_speed     */ 0.0f,
    /* p_duty          */ 0.50f,
    /* p_duty_ramp     */ 0.0f,
    /* p_repeat_speed  */ 0.55f,
    /* p_pha_offset    */ 0.0f,
    /* p_pha_ramp      */ 0.0f,
    /* p_lpf_freq      */ 1.0f,
    /* p_lpf_ramp      */ 0.0f,
    /* p_lpf_resonance */ 0.0f,
    /* p_hpf_freq      */ 0.0f,
    /* p_hpf_ramp      */ 0.0f,
    /* sound_vol       */ 1.0f,
};

// "thruster": low, filtered noise rumble with slow vibrato — engine loop-ish.
static const sfxr_params sfxThruster = {
    /* wave_type       */ 3,
    /* p_env_attack    */ 0.10f,
    /* p_env_sustain   */ 0.50f,
    /* p_env_punch     */ 0.0f,
    /* p_env_decay     */ 0.25f,
    /* p_base_freq     */ 0.18f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ 0.0f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.15f,
    /* p_vib_speed     */ 0.40f,
    /* p_arp_mod       */ 0.0f,
    /* p_arp_speed     */ 0.0f,
    /* p_duty          */ 0.50f,
    /* p_duty_ramp     */ 0.0f,
    /* p_repeat_speed  */ 0.0f,
    /* p_pha_offset    */ 0.0f,
    /* p_pha_ramp      */ 0.0f,
    /* p_lpf_freq      */ 0.45f,
    /* p_lpf_ramp      */ 0.0f,
    /* p_lpf_resonance */ 0.20f,
    /* p_hpf_freq      */ 0.0f,
    /* p_hpf_ramp      */ 0.0f,
    /* sound_vol       */ 1.0f,
};

// "powerup": rising sine sweep with vibrato — collectible / level-up.
static const sfxr_params sfxPowerup = {
    /* wave_type       */ 2,
    /* p_env_attack    */ 0.0f,
    /* p_env_sustain   */ 0.30f,
    /* p_env_punch     */ 0.0f,
    /* p_env_decay     */ 0.40f,
    /* p_base_freq     */ 0.25f,
    /* p_freq_limit    */ 0.0f,
    /* p_freq_ramp     */ 0.22f,
    /* p_freq_dramp    */ 0.0f,
    /* p_vib_strength  */ 0.35f,
    /* p_vib_speed     */ 0.45f,
    /* p_arp_mod       */ 0.0f,
    /* p_arp_speed     */ 0.0f,
    /* p_duty          */ 0.50f,
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

// Table of all effects (indexable / cyclable from the main loop).
static const sfxr_params *const sfxTable[] = {
    &sfxBlip,
    &sfxCoin,
    &sfxJump,
    &sfxHurt,
    &sfxShoot,
    &sfxSelect,
    &sfxError,
    &sfxAlarm,
    &sfxThruster,
    &sfxPowerup,
};
static const int sfxCount = sizeof(sfxTable) / sizeof(sfxTable[0]);

static sfxr_state sfx;
// Index into sfxTable[] to (re)start on the audio thread, or -1 for none.
// Set from the main loop when a key is pressed; consumed in audioLoop().
static volatile int sfxTrigger = -1;

static const bool USE_C_SCALE = false;

static uint32_t t = 0;
void audioLoop(int16_t* ptr, int count) {
    // Pick up a pending trigger from the main loop and (re)start the effect.
    int trig = sfxTrigger;
    if (trig >= 0 && trig < sfxCount) {
        sfxTrigger = -1;
        sfxr_reset(&sfx, sfxTable[trig]);
    }

    for (int i = 0; i < count; ++i) {

        // Downsample t to 8kHz regardless of your actual sample rate
        // e.g. at 44100Hz: t increments ~every 5.5 samples
        // Use a accumulator to get the right effective rate
        static uint32_t acc = 0;
        acc += 8000;
        if (acc >= KSM_SAMPLE_RATE) {
            acc -= KSM_SAMPLE_RATE;
            t++;
        }


        int16_t sample = 0;

        // SFXR effect takes priority over the background synth while playing.
        if (sfxr_playing(&sfx)) {
            float f;
            sfxr_generate(&sfx, &f, 1);
            sample = (int16_t)(f * 32000.0f);
            
        }
		*ptr++ = sample;
        *ptr++ = sample;
		continue;

        if (USE_C_SCALE) {
            float freq = SCALE_FREQS[currentNote];
            float phaseInc = freq * float(PHASE_SIZE) / float(KSM_SAMPLE_RATE);
            phaseIndex += phaseInc;
            if (phaseIndex >= float(PHASE_SIZE))
                phaseIndex -= float(PHASE_SIZE);
            sample = phaseTable[int(phaseIndex)];

            if (++noteSampleCount >= NOTE_DURATION_SAMPLES) {
                noteSampleCount = 0;
                currentNote = (currentNote + 1) % NOTE_COUNT;
            }

            *ptr++ = sample;
            *ptr++ = sample;
            continue;
        }

        uint8_t s = 128;
		switch(musicSelect) {
			case 1: s = t*((t&4096?t%65536<59392?7:t>>6:16)+(1&t>>14))>>(3&-t>>(t&2048?2:10))|t>>(t&16384?t&4096?4:3:2);
			break;
        	case 2: s = t*(2&t>>13?7:5)*(3-(3&t>>9)+(3&t>>8))>>(3&-t>>(t&4096|(t>>11)%32>28?2:16))|t>>3;
			break;
			case 3: s = (~t>>2)*((127&t*(7&t>>10))<(245&t*(2+(5&t>>14))));
			break;
			// case 4: s = (t&4096?(t*(t^t%255)|t>>4)>>1:t>>3|(t&8192?t<<2:t))^((t&8192?t&4096?t&1024?2*t:4*t:t&512?4*t:4.2*t:(t&4096?t&1024?2*t:10*t:t&512?2*t:8*t)>>2)*(t&16384?3:2)|t*(t&16384?1/8:1/(.01*t))>>1);
			// break;
			case 5: s = t*(t>>(t&4096?t*t>>12:t>>12))|t<<(t>>8)|t>>4;
			break;
			case 6: s = t>>5|t>>4|(t%42*(t>>4)|357052691-(t>>4))/(t>>16)^(t|t>>4);
			break;
			case 7: s = t/((t>>3-(t>>14)%2)%(26>>(t>>16)%3))%1024/12<<9/(t>>5&127);
			break;
			case 8: s = (t>>10^t>>11)%5*((t>>14&3^t>>15&1)+1)*t%99+((3+(t>>14&3)-(t>>16&1))/3*t%99&64);
			break;
			case 9: s = ((t*(t>>12))<<(-t>>10&7))&-t>>2;
			break;
			case 10: s = (t%125&t>>8)|t>>4|t*t>>8&t>>8;
			break;
			default: break;
		}

        // Scale uint8 [0..255] to int16 [-32768..32767]
        sample = ((int16_t)s - 128) << 8;

        *ptr++ = sample;
        *ptr++ = sample; // stereo
    }
}

// Some platforms need an emulated unplug/plug event to refresh composite descriptors
void refreshUSBDescriptors()
{
	TinyUSBDevice.detach();
  	// If needed, a small delay can be added here
  	TinyUSBDevice.attach();
}

void setup() {
	// If waking from System OFF, require 1s hold to boot — same threshold as shutdown.
	// // Anything shorter goes back to sleep without initializing hardware.
	// if (NRF_POWER->RESETREAS & POWER_RESETREAS_OFF_Msk) {
	// 	NRF_POWER->RESETREAS = 0xFFFFFFFF;
	// 	pinMode(PIN_MENU, INPUT_PULLUP);
	// 	bool held = (digitalRead(PIN_MENU) == LOW);
	// 	for (int i = 0; i < 1000 && held; i++) {
	// 		delay(1);
	// 		held = (digitalRead(PIN_MENU) == LOW);
	// 	}
	// 	if (!held) {
	// 		NRF_GPIO->PIN_CNF[PIN_MENU] = (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos)
	// 									 | (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos)
	// 									 | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
	// 									 | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
	// 		NRF_POWER->SYSTEMOFF = 1;
	// 		__DSB();
	// 		while(true) {}
	// 	}
	// }

	// Serial.begin(115200);
	ksm::init();
	colordemo::init();
	initPhaseTable();
	ksm::setupAudio(audioLoop);
	IMU.begin();
	// usbHID.begin();
	// blemidi.begin();
	ble::init();
	ble::advertise(true);
	refreshUSBDescriptors();

	// MIDI.begin(MIDI_CHANNEL_OMNI);


	// TODO: this callback isn't great
	// Should we handle usbHID internally entirely ?
	// ksm::setKeyboardReportCallback([](uint8_t modifiers, uint8_t* keys) {
	// 	usbHID.keyboardReport(0, modifiers, keys);
	// 	bleHID.keyboardReport(modifiers, keys);
	// });
}


int keyDown = 0;
void loop() {
	ksm::loop();

	// // Wait for USB to be ready before sending reports
	// if (!TinyUSBDevice.mounted()) {
	// 	return;
	// }

	auto kup = ksm::keycodes::ARROW_UP;
	auto kdown = ksm::keycodes::ARROW_DOWN;
	auto kleft = ksm::keycodes::ARROW_LEFT;
	auto kright = ksm::keycodes::ARROW_RIGHT;
	auto kenter = ksm::keycodes::ENTER;
	auto kspace = ksm::keycodes::SPACE;
	auto kalt = ksm::keycodes::ALT_LEFT;
	auto ktab = ksm::keycodes::TAB;

	int keyMapping[] = {
		ktab,		kup,	0,		0,			0,
		kleft,	kdown,	kright,	kspace,		kalt,
	};

	if(ksm::press(1)) {
		int l = ksm::getBatLevel();
		Serial.printf("Bat: %d\n", l);

		// int vbat = analogRead(PIN_BAT_LVL);
		int vcharge = analogRead(PIN_CHG);
		bool isCharging = digitalRead(PIN_CHG) == LOW;

		// int vusb = analogRead(PIN_USB_ST);
		// Serial.printf("bat: %d\n", vbat);
		Serial.printf("charge: %i\n", isCharging);
		// Serial.printf("usb: %d\n", vusb);
	}

	// ksm::setColor(1, 0x000050);
	ksm::setColor(0, 0x00FF00);


	// int charge = digitalRead(PIN_CHG);
	// int usb = digitalRead(PIN_USB_ST);

	// ksm::setColor(3, usb == HIGH ? 0x500000 : 0);


	ksm::setRumble(ksm::down(2));
	// ksm::setColor(2, ksm::down(2) ? 0x500000 : 0);


 	for(int i=1; i<=10; ++i) {
		 if(ksm::press(i)) {
			// MIDI.sendNoteOn(i + 60, 100, 1);
			musicSelect = i;
			// Each key plays its own sound effect: key i -> sfxTable[i-1].
			// sfxTrigger = i - 1;
		}
		if(ksm::release(i)) {
			// MIDI.sendNoteOff(i + 60, 0, 1);
		}
	}

	// Color / gradient / pattern demo owns the 10 key LEDs. Any key press
	// advances to the next visual mode; each mode paints all 10 keys.
	colordemo::update();
}
