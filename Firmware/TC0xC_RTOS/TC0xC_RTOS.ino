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
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // const TaskHandle_t tasks_to_alert[] = {Badge.leds.task_handle};

    uint8_t new_presses = 0;
    uint8_t new_releases = 0;
    get_button_states(&new_presses, &new_releases);
    if (new_presses)
    {
        Badge.cli.serial->println("test");
        xTaskNotify(Badge.leds.task_handle, 1, eSetValueWithOverwrite);
    }

    vTaskDelay(100);
}
