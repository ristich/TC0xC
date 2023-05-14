#include <Arduino.h>
#include "rtos.h"
#include "touch.h"
#include "audio.h"

static uint8_t touch_buffer[KONAMI_LENGTH] = {0};
static uint8_t touch_buffer_index = 0;

void touch_task(void *pvParameters);
uint8_t is_touched(uint8_t gpioPin, uint8_t threshold);
uint8_t get_button_states(uint8_t *new_presses, uint8_t *new_releases);
bool check_Konami();

Touch_Error touch_init(Touch_Object *touch, CLI_Object *cli, TaskHandle_t led_handle, TaskHandle_t audio_handle)
{
    if (touch->initialized)
        return TOUCH_SUCCESS;

    touch->cli = cli;
    touch->led_handle = led_handle;
    touch->audio_handle = audio_handle;

    xTaskCreatePinnedToCore(touch_task, "touch_task", 2048, touch, tskIDLE_PRIORITY + 2, NULL, app_cpu);

    touch->initialized = true;

    return TOUCH_SUCCESS;
}

void touch_task(void *pvParameters)
{
    Touch_Object *touch = (Touch_Object *)pvParameters;

    uint8_t new_presses = 0;
    uint8_t new_releases = 0;

    while (1)
    {
        get_button_states(&new_presses, &new_releases);
        if (new_presses)
        {
            xTaskNotifyIndexed(touch->led_handle, 0, LED_BUTTON_PRESS, eSetValueWithoutOverwrite);
            for (uint8_t i = 0; i < TOTAL_BUTTONS; i++)
            {
                if (new_presses & Buttons[i].mask)
                {
                    xTaskNotifyIndexed(touch->audio_handle, 0, (i + 1), eSetValueWithoutOverwrite);
                    touch->cli->serial->println(Buttons[i].name);
                }
            }
            if (check_Konami())
            {
                xTaskNotifyIndexed(touch->audio_handle, 0, STAGE_COMPLETE_SONG, eSetValueWithOverwrite);
                // todo: flag
                touch->cli->serial->println("Hadouken! insert flag here");
            }
        }

        vTaskDelay(100);
    }
}

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
        if (is_touched(Buttons[i].pin, TOUCH_THRESH))
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
                    {
                        touch_buffer_index = 0;
                    }
                }
                else
                {
                    *new_releases |= (1 << i);
                }
            }
        }
    }

    previous_presses = current_presses;
    return current_presses;
}

/**
 * @brief checks to see if the last sequence of button presses was the
 *  Konami code
 *
 * @note it is up to the programmer to call this after a new press is
 *  registered
 *
 * @return true Konami code was entered
 * @return false Konami code was not entered
 */
bool check_Konami()
{
    const static uint8_t konami_buffer[KONAMI_LENGTH] = {
        UP_BIT,
        UP_BIT,
        DOWN_BIT,
        DOWN_BIT,
        LEFT_BIT,
        RIGHT_BIT,
        LEFT_BIT,
        RIGHT_BIT,
        SELECT_BIT,
    };

    bool is_konami = true;
    for (uint8_t i = 0; i < KONAMI_LENGTH; i++)
    {
        uint8_t check_index = (touch_buffer_index + i) % KONAMI_LENGTH;
        if (touch_buffer[check_index] != konami_buffer[i])
        {
            is_konami = false;
        }
    }

    return is_konami;
}