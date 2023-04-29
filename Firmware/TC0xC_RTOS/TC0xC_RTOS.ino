#include "rtos.h"
#include "led.h"
#include "cli.h"

#define I2C_ADDR 0x74
#define I2C_SDA 23u
#define I2C_SCL 18u
#define LED_SDB 22u

TC_IS31FL3731 led_controller = TC_IS31FL3731();

// static SemaphoreHandle_t state_mutex;
// static SemaphoreHandle_t state_sem;
// TaskHandle_t Handle_Task_CLI;
// TaskHandle_t Handle_Task_LEDs;

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

  led_controller.begin(I2C_SDA, I2C_SCL);

  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // BadgeData bd = {
  //   .led_mode = LED_MODE_RESET,
  // };

  // state_sem = xSemaphoreCreateBinary();
  // state_mutex = xSemaphoreCreateMutex();

  // task initializations
  LED_init(&led_controller);
  CLI_init(&Serial);
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
