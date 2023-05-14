#include "rtos.h"
#include "led.h"
#include "cli.h"
#include "touch.h"
#include "audio.h"
#include <EEPROM.h>

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
    // check for dev mode
    EEPROM.begin(EEPROM_SIZE);
    delay(500);
    uint8_t dev_read = EEPROM.readByte(EEPROM_ADDR_DEV_MODE);
    if (dev_read > 1)
    {
        EEPROM.writeByte(EEPROM_ADDR_DEV_MODE, 0);
        EEPROM.commit();
    }
    else if (dev_read == 1)
    {
        Badge.cli.dev_mode = 1;
    }

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
