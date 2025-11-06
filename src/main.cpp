#include <Arduino.h>
#include "keysmet.h"


void setup() {
	ksm_init();
}

void loop() {
	for(int i=1; i<=10; ++i) {
		if(hold(i, 1000)) {
			setColor(i, 0x200000);
		}
	}

	ksm_loop();
}
