#include "rtos.h"
#include "led.h"
#include "cli.h"

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
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
