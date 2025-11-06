

#ifndef _VARIANT_KSM1_
#define _VARIANT_KSM1_

/** Master clock frequency */
#define VARIANT_MCK       (64000000ul)

// #define USE_LFXO      // Board uses 32khz crystal for LF
#define USE_LFRC    // Board uses RC for LF

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

// Number of pins defined in PinDescription array
#define PINS_COUNT           (48)
#define NUM_DIGITAL_PINS     (48)
#define NUM_ANALOG_INPUTS    (6)
#define NUM_ANALOG_OUTPUTS   (0)


#define LED_BUILTIN          (32+11) // NC
#define LED_CONN             (32+12) // NC

#define LED_RED              (32+12) // NC
#define LED_BLUE             (32+11) // NC, used for BLE connection status

#define LED_STATE_ON         0         // State when LED is lit

#define ADC_RESOLUTION    14

// Arduino Header D0, D1
#define PIN_SERIAL1_RX      (32+13) // NC
#define PIN_SERIAL1_TX      (32+14) // NC

#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (4)
#define PIN_WIRE_SCL         (11)



#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------------------------
 *        Arduino objects - C++ only
 *----------------------------------------------------------------------------*/

#endif
