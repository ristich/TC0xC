#pragma once

#include "hal.h"

// touch button tresholds
#define TOUCH_THRESH 25
#define KONAMI_LENGTH 9

typedef enum buttons
{
    DOWN_BUTTON = 0,
    LEFT_BUTTON,
    UP_BUTTON,
    RIGHT_BUTTON,
    SELECT_BUTTON,
} buttons;

const uint8_t Button_Pins[TOTAL_BUTTONS]{DOWN_TOUCH, LEFT_TOUCH, UP_TOUCH, RIGHT_TOUCH, SELECT_TOUCH};

uint8_t get_button_states(uint8_t *new_presses, uint8_t *new_releases);
bool checkKonami();