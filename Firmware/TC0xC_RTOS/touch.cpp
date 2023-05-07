#include <Arduino.h>
#include "rtos.h"
#include "touch.h"

static uint8_t touch_buffer[KONAMI_LENGTH] = {0};
static uint8_t touch_buffer_index = 0;

/**
 * @brief simple debounce check for touch sensing
 *
 * @param gpioPin gpio pin number or touch number to check
 * @param threshold threshold to compare against
 * @return uint8_t 1: pressed, 0: released
 */
uint8_t is_touched(uint8_t gpioPin, uint8_t threshold)
{
    if (touchRead(gpioPin) <= threshold)
    {
        if (touchRead(gpioPin) <= threshold)
        {
            if (touchRead(gpioPin) <= threshold)
            {
                return 1;
            }
        }
    }

    return 0;
}

/**
 * @brief Get the current states of touch buttons
 *
 * @note uses the bits of the uint8_t to represent buttons, and it follows the
 *  order of enum buttons for bit order (bit 0 is down, bit 1 is left, etc), so
 *  if you want to check the up button's state, you would check bit 2. If the
 *  returned number was 10 (0b00001010), the would mean left and right buttons
 *  are currently pressed.
 *
 * @param new_presses cleared with each call and bits are set for any button
 *  that has been pressed since the last call
 * @param new_releases cleared with each call and bits are set for any button
 *  that has been released since the last call
 * @return uint8_t the current state of the touch buttons. Bits are set for
 *  buttons that are pressed and cleared for buttons that are not pressed
 */
uint8_t get_button_states(uint8_t *new_presses, uint8_t *new_releases)
{
    static uint8_t previous_presses = 0;
    uint8_t current_presses = 0;
    *new_presses = 0;
    *new_releases = 0;

    for (uint8_t i = 0; i < TOTAL_BUTTONS; i++)
    {
        if (is_touched(Button_Pins[i], TOUCH_THRESH))
        {
            current_presses |= (1 << i);
        }
    }

    uint8_t changed_presses = previous_presses ^ current_presses;
    if (changed_presses)
    {
        for (uint8_t i = 0; i < TOTAL_BUTTONS; i++)
        {
            if ((changed_presses >> i) & 0x1)
            {
                if ((current_presses >> i) & 0x1)
                {
                    *new_presses |= (1 << i);

                    touch_buffer[touch_buffer_index] = i;
                    touch_buffer_index++;
                    if (touch_buffer_index >= KONAMI_LENGTH)
                        touch_buffer_index = 0;
                }
                else
                    *new_releases |= (1 << i);
            }
        }
    }

    previous_presses = current_presses;
    return current_presses;
}

bool check_Konami()
{
    const static uint8_t konami_buffer[KONAMI_LENGTH] = {
        UP_BUTTON,
        UP_BUTTON,
        DOWN_BUTTON,
        DOWN_BUTTON,
        LEFT_BUTTON,
        RIGHT_BUTTON,
        LEFT_BUTTON,
        RIGHT_BUTTON,
        SELECT_BUTTON,
    };

    bool is_konami = true;
    for (uint8_t i = 0; i < KONAMI_LENGTH; i++)
    {
        uint8_t check_index = (touch_buffer_index + i) % KONAMI_LENGTH;
        if (touch_buffer[check_index] != konami_buffer[i])
            is_konami = false;
    }

    return is_konami;
}