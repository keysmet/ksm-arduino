#include <Arduino.h>
#include "keysmet.h"


void setup() {
	ksm_init();
}

void loop() {
	for(int i=1; i<=10; ++i) {
		if(isDown(i)) {
			setColor(i, 0x200000);
		} else {
			setColor(i, 0);
		}
	}

	ksm_loop();
}
