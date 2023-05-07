#pragma once

#include "TC_IS31FL3731.h"

#define I2C_ADDR 0x74

typedef enum LED_Error
{
    LED_SUCCESS = 0,
} LED_Error;

typedef enum
{
    LED_MODE_OFF = 0,
    LED_MODE_ROTATE,
    LED_MODE_BLINK,
    LED_MODE_RESET,
} led_mode_t;

// led object to be passed around for led control
typedef struct LED_Object
{
    bool initialized;             // state of led task creation
    TaskHandle_t task_handle;     // task handle
    hw_timer_t *message_timer;    // alarm for detecting when to show message
    SemaphoreHandle_t update_sem; // alert for new value updates
    TC_IS31FL3731 *controller;    // pointer to serial interface
    led_mode_t mode;              // current set mode
    uint16_t delay_ms;            // delay between LED changes
    uint8_t brightness;           // brightness of LEDs
} LED_Object;

LED_Error LED_init(LED_Object *leds);
