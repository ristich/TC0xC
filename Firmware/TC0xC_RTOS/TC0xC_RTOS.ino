#include "rtos.h"
#include "led.h"
#include "cli.h"

// static SemaphoreHandle_t state_mutex;
// static SemaphoreHandle_t state_sem;

typedef struct
{
    LED_Object leds;
    CLI_Object cli;
} Badge_Object;

Badge_Object Badge = {0};

void setup()
{
    // state_sem = xSemaphoreCreateBinary();
    // state_mutex = xSemaphoreCreateMutex();

    // task initializations
    LED_init(&Badge.leds);
    CLI_init(&Badge.cli, &Badge.leds);
}

void loop()
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
