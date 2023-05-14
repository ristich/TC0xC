#pragma once

#include "TC_IS31FL3731.h"

#define I2C_ADDR 0x74

static const uint8_t LED_MAX_BRIGHTNES = 0x80;
static const uint16_t LED_MAX_DELAY_ms = 693;

typedef enum LED_Error
{
    LED_SUCCESS = 0,
} LED_Error;

typedef enum led_mode_t
{
    LED_MODE_OFF = 0,
    LED_MODE_ROTATE_CW,
    LED_MODE_ROTATE_CCW,
    LED_MODE_BLINK,
    LED_MODE_RINGS_OUT,
    LED_MODE_RINGS_IN,
    LED_MODE_TOTAL,
} led_mode_t;

const led_mode_t Led_Mode_Default = LED_MODE_ROTATE_CW;
const uint16_t Led_Delay_Default = 500;
const uint8_t Led_Brightness_Default = 25;

typedef enum LED_Event
{
    LED_UPDATE = 0,
    LED_BUTTON_PRESS,
} LED_Event;

// led object to be passed around for led control
typedef struct LED_Object
{
    bool initialized;             // state of led task creation
    TaskHandle_t task_handle;     // task handle
    hw_timer_t *message_timer;    // alarm for detecting when to show message
    TC_IS31FL3731 *controller;    // pointer to serial interface
    led_mode_t mode;              // current set mode
    uint16_t delay_ms;            // delay between LED changes
    uint8_t brightness;           // brightness of LEDs
} LED_Object;

LED_Error LED_init(LED_Object *leds);
