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
    xSemaphoreTake(Badge.leds.event_sem, portMAX_DELAY);
}

void loop()
{
    // vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Badge.cli.serial->print(".");
    xSemaphoreTake(Badge.leds.event_sem, 0);
    if(touchInterrupt())
    {
        Badge.cli.serial->println("test");
        xSemaphoreGive(Badge.leds.event_sem);
    }

    vTaskDelay(100);
}

// simple debounce check for touch sensing
//   gpioPin: gpio or touch number
//   threshold: threshold for touchRead
uint8_t checkTouch(uint8_t gpioPin, uint8_t threshold) {
  if (touchRead(gpioPin) <= threshold) {
    if (touchRead(gpioPin) <= threshold) {
      if (touchRead(gpioPin) <= threshold) {
        return 1;
      }
    }
  }

  return 0;
}

bool touchInterrupt()
{
    // if ((checkTouch(DOWN_TOUCH, TOUCH_THRESH)) || (checkTouch(LEFT_TOUCH, TOUCH_THRESH)) ||
    //     (checkTouch(UP_TOUCH, TOUCH_THRESH)) || (checkTouch(RIGHT_TOUCH, TOUCH_THRESH)) ||
    //     (checkTouch(SELECT_TOUCH, TOUCH_THRESH)))
    if (touchRead(SELECT_TOUCH) <= TOUCH_THRESH)
    {
        return true;
    }

    return false;
}