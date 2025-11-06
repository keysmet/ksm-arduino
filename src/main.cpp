#include <Arduino.h>
#include "keysmet.h"

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
}

void loop() {
	for(int i=1; i<=10; ++i) {
		if(press(i)) {
			setColor(i, 0x200000);
		}
	}

	ksm_loop();
}
