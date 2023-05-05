#include "rtos.h"
#include "led.h"
#include "cli.h"
#include "sound.h"

typedef struct
{
    LED_Object leds;
    CLI_Object cli;
} Badge_Object;

Badge_Object Badge = {0};

void setup()
{
    // task initializations
    pinMode(BUZZER_PIN,OUTPUT);
    LED_init(&Badge.leds);
    CLI_init(&Badge.cli, &Badge.leds);
    open(BUZZER_PIN);
}

void loop()
{
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
