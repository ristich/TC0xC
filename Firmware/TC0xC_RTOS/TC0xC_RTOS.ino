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
    Touch_Object touch;
} Badge_Object;

Badge_Object Badge = {0};

void setup()
{
    // task initializations
    LED_init(&Badge.leds);
    audio_init(&Badge.audio);
    CLI_init(&Badge.cli, &Badge.leds);
    touch_init(&Badge.touch, &Badge.cli, Badge.leds.task_handle, Badge.audio.task_handle);
}

void loop()
{
    vTaskDelay(100);
}
