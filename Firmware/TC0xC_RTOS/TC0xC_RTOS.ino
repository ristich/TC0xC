#include "rtos.h"
#include "TC_IS31FL3731.h"
#include "cli.h"

#define I2C_ADDR 0x74
#define I2C_SDA 23u
#define I2C_SCL 18u
#define LED_SDB 22u

TC_IS31FL3731 ledController = TC_IS31FL3731();

const TickType_t Delay_ms = 500;
const uint32_t Brightness = 10;

// static SemaphoreHandle_t state_mutex;
// static SemaphoreHandle_t state_sem;
// TaskHandle_t Handle_Task_CLI;
// TaskHandle_t Handle_Task_LEDs;

typedef enum
{
  LED_MODE_OFF = 0,
  LED_MODE_ROTATE,
  LED_MODE_BURST,
  LED_MODE_RESET,
} led_mode_t;

typedef struct
{
  led_mode_t led_mode;
} BadgeData;

void setup()
{
  // pin initializations
  pinMode(I2C_SDA, OUTPUT);
  pinMode(I2C_SCL, OUTPUT);
  pinMode(LED_SDB, OUTPUT);
  digitalWrite(LED_SDB, HIGH);

  ledController.begin(I2C_SDA, I2C_SCL);

  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // BadgeData bd = {
  //   .led_mode = LED_MODE_RESET,
  // };

  // state_sem = xSemaphoreCreateBinary();
  // state_mutex = xSemaphoreCreateMutex();

  // task initializations
  CLI_init(&Serial);
  xTaskCreatePinnedToCore(Task_LEDs, "Task_LEDs", 2048, NULL, tskIDLE_PRIORITY + 2, NULL, app_cpu);
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void Task_LEDs(void *pvParameters)
{
  Serial.println("LED Task");
  while (1)
  {
    set_LED_mode(LED_MODE_ROTATE);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void set_LED_mode(led_mode_t mode)
{
  static led_mode_t prev_mode = LED_MODE_RESET;
  static TickType_t start_time_ms = 0;
  static uint8_t mode_step = 0;

  if (mode != prev_mode)
  {
    start_time_ms = xTaskGetTickCount() - Delay_ms;
    mode_step = 0;
  }

  switch (mode)
  {
  case LED_MODE_OFF:
  {
    ledController.setAllLEDPWM(0);

    break;
  }

  case LED_MODE_ROTATE:
  {
    if (xTaskGetTickCount() > (start_time_ms + Delay_ms))
    {
      if (mode_step == 0)
      {
        ledController.setAllLEDPWM(0);
        ledController.setColumn(LED_COL_TWELVE, Brightness);
      }
      else if (mode_step == 1)
      {
        ledController.setAllLEDPWM(0);
        ledController.setColumn(LED_COL_ONE, Brightness);
      }
      else if (mode_step == 2)
      {
        ledController.setAllLEDPWM(0);
        ledController.setColumn(LED_COL_FIVE, Brightness);
      }
      else if (mode_step == 3)
      {
        ledController.setAllLEDPWM(0);
        ledController.setColumn(LED_COL_SIX, Brightness);
      }
      else if (mode_step == 4)
      {
        ledController.setAllLEDPWM(0);
        ledController.setColumn(LED_COL_SEVEN, Brightness);
      }
      else if (mode_step == 5)
      {
        ledController.setAllLEDPWM(0);
        ledController.setColumn(LED_COL_ELEVEN, Brightness);
      }
      else if (mode_step == 6)
      {
        mode = LED_MODE_RESET;
      }

      start_time_ms += Delay_ms;
      mode_step++;
    }

    break;
  }

  default:
  {
    ledController.setAllLEDPWM(0);

    break;
  }
  }
  prev_mode = mode;
}
