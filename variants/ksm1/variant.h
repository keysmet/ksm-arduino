

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



/*----------------------------------------------------------------------------
 *        KSM1 Hardware Pin Definitions
 *----------------------------------------------------------------------------*/

#define PINID(r, i) ((r) * 32 + (i))

#define KSM_SAMPLE_RATE 22727.27f

enum PIN {
    PIN_MENU        = PINID(1,10),
    PIN_MENU_LED    = PINID(0,28),
    PIN_BAT_LVL     = PINID(0,30),
    PIN_USB_ST      = PINID(0,29),
    PIN_CHG         = PINID(0,31),
    PIN_K10         = PINID(0,27),
    PIN_LED         = PINID(0,0),
    PIN_I2S_LRCK    = PINID(0,1),
    PIN_I2S_DIN     = PINID(0,26),
    PIN_I2C_SDA     = PINID(0,4),
    PIN_I2S_BCK     = PINID(0,5),
    PIN_K5          = PINID(0,6),
    PIN_PWR_LED     = PINID(0,7),
    PIN_K4          = PINID(0,8),
    PIN_K9          = PINID(1,9),
    PIN_I2C_SCL     = PINID(0,11),
    PIN_PWR_ON      = PINID(0,25),
    PIN_K6          = PINID(0,24),
    PIN_K1          = PINID(0,22),
    PIN_K7          = PINID(0,20),
    PIN_VIB         = PINID(0,21),
    PIN_K2          = PINID(0,17),
    PIN_K8          = PINID(0,15),
    PIN_K3          = PINID(0,13),
};

enum KEY {
    KEY_MENU = 0,
    KEY_1 = 1,
    KEY_2 = 2,
    KEY_3 = 3,
    KEY_4 = 4,
    KEY_5 = 5,
    KEY_6 = 6,
    KEY_7 = 7,
    KEY_8 = 8,
    KEY_9 = 9,
    KEY_10 = 10,
    KEY_COUNT = 11,
};

#ifdef __cplusplus
}
#endif

#endif
