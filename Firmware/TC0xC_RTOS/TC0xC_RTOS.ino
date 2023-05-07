#include "rtos.h"
#include "led.h"
#include "cli.h"
#include "touch.h"

typedef struct
{
    LED_Object leds;
    CLI_Object cli;
} Badge_Object;

Badge_Object Badge = {0};

void setup()
{
    // task initializations
    LED_init(&Badge.leds);
    CLI_init(&Badge.cli, &Badge.leds);
}

void loop()
{
    uint8_t new_presses = 0;
    uint8_t new_releases = 0;
    get_button_states(&new_presses, &new_releases);
    if (new_presses)
    {
        xTaskNotify(Badge.leds.task_handle, 1, eSetValueWithOverwrite);
        if (new_presses & 0x1)
            Badge.cli.serial->println("down");
        if (new_presses & 0x2)
            Badge.cli.serial->println("left");
        if (new_presses & 0x4)
            Badge.cli.serial->println("up");
        if (new_presses & 0x8)
            Badge.cli.serial->println("right");
        if (new_presses & 0x10)
            Badge.cli.serial->println("select");
        if (check_Konami())
            // todo: flag
            Badge.cli.serial->println("Hadouken!");
    }

    vTaskDelay(100);
}
