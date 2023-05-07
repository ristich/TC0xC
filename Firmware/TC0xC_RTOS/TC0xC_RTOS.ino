#include "rtos.h"
#include "led.h"
#include "cli.h"
#include "touch.h"
#include "audio.h"

typedef struct
{
    LED_Object leds;
    CLI_Object cli;
    Audio_Object audio;
} Badge_Object;

Badge_Object Badge = {0};

void setup()
{
    // task initializations
    LED_init(&Badge.leds);
    audio_init(&Badge.audio);
    CLI_init(&Badge.cli, &Badge.leds);
}

void loop()
{
    uint8_t new_presses = 0;
    uint8_t new_releases = 0;
    get_button_states(&new_presses, &new_releases);
    if (new_presses)
    {
        xTaskNotifyIndexed(Badge.leds.task_handle, 0, 0, eSetValueWithoutOverwrite);
        xTaskNotifyIndexed(Badge.audio.task_handle, 0, 0, eSetValueWithoutOverwrite);
        for (uint8_t i = 0; i<TOTAL_BUTTONS; i++)
        {
            if (new_presses & Buttons[i].mask)
                Badge.cli.serial->println(Buttons[i].name);
        }
        if (check_Konami())
            // todo: flag
            Badge.cli.serial->println("Hadouken!");
    }

    vTaskDelay(100);
}
