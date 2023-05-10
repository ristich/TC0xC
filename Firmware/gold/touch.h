#pragma once

#include "cli.h"
#include "hal.h"

// touch button tresholds
#define TOUCH_THRESH 30
#define KONAMI_LENGTH 9

typedef enum Touch_Error
{
    TOUCH_SUCCESS = 0,
} Touch_Error;

typedef enum Button_Bit
{
    DOWN_BIT = 0,
    LEFT_BIT,
    UP_BIT,
    RIGHT_BIT,
    SELECT_BIT,
    TOTAL_BUTTONS,
} Button_Bit;

typedef struct Touch_Button
{
    const uint8_t pin;    // GPIO pin number
    const Button_Bit bit; // bit number
    const uint8_t mask;   // bit mask
    const char *name;     // name of button
} Touch_Button;

const Touch_Button Buttons[TOTAL_BUTTONS]{
    {DOWN_TOUCH, DOWN_BIT, (1 << DOWN_BIT), "Down"},
    {LEFT_TOUCH, LEFT_BIT, (1 << LEFT_BIT), "Left"},
    {UP_TOUCH, UP_BIT, (1 << UP_BIT), "Up"},
    {RIGHT_TOUCH, RIGHT_BIT, (1 << RIGHT_BIT), "Right"},
    {SELECT_TOUCH, SELECT_BIT, (1 << SELECT_BIT), "Select"},
};

typedef struct Touch_Object
{
    bool initialized;
    TaskHandle_t led_handle;
    TaskHandle_t audio_handle;
    CLI_Object *cli;
} Touch_Object;

Touch_Error touch_init(Touch_Object *touch, CLI_Object *cli, TaskHandle_t led_handle, TaskHandle_t audio_handle);
