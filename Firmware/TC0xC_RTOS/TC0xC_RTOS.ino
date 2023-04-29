#include "rtos.h"
#include "led.h"
#include "cli.h"

// static SemaphoreHandle_t state_mutex;
// static SemaphoreHandle_t state_sem;
// TaskHandle_t Handle_Task_CLI;
// TaskHandle_t Handle_Task_LEDs;

typedef struct
{
  LED_Object leds;
  CLI_Object cli;
} Badge_Object;

Badge_Object Badge = {0};

void setup()
{
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // state_sem = xSemaphoreCreateBinary();
  // state_mutex = xSemaphoreCreateMutex();

  // task initializations
  LED_init(&Badge.leds);
  CLI_init(&Serial);
}

void loop()
{
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
