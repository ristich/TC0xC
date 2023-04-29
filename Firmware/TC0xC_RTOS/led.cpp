#include "led.h"

static void LED_task(void *pvParameters);
void set_LED_mode(LED_Object *leds);

static void rotate_leds(LED_Object *leds, TickType_t time);

TC_IS31FL3731 led_controller = TC_IS31FL3731();

LED_Error LED_init(LED_Object *leds)
{
    static bool initialized = false;
    if (initialized)
    {
        return LED_SUCCESS;
    }

    // init pins for led driver
    pinMode(I2C_SDA, OUTPUT);
    pinMode(I2C_SCL, OUTPUT);
    pinMode(LED_SDB, OUTPUT);
    digitalWrite(LED_SDB, HIGH);

    // init led driver
    led_controller.begin(I2C_SDA, I2C_SCL);
    leds->controller = &led_controller;

    // init led setting defaults
    leds->mode = LED_MODE_ROTATE;
    leds->delay_ms = 500;
    leds->brightness = 10;

    xTaskCreatePinnedToCore(LED_task, "LED_task", 2048, leds, tskIDLE_PRIORITY + 2, NULL, app_cpu);

    return LED_SUCCESS;
}

void LED_task(void *pvParameters)
{
    LED_Object *leds = (LED_Object *)pvParameters;
    // (void)leds;

    while (1)
    {
        // todo: fix this so it's not updating the LED driver
        // when it doesn't need to
        set_LED_mode(leds);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void set_LED_mode(LED_Object *leds)
{
    TickType_t uptime_ms = xTaskGetTickCount();

    switch (leds->mode)
    {
    case LED_MODE_OFF:
        leds->controller->setAllLEDPWM(0);
        break;

    case LED_MODE_ROTATE:
        rotate_leds(leds, uptime_ms);
        break;

    default:
        leds->controller->setAllLEDPWM(0);
        break;
    }
}

static void rotate_leds(LED_Object *leds, TickType_t time)
{
    static const uint8_t max_steps = 6;
    static uint8_t step = 0;
    static TickType_t update_time_ms = 0;
    static const led_col_t col_order[max_steps] = {
        LED_COL_TWELVE,
        LED_COL_ONE,
        LED_COL_FIVE,
        LED_COL_SIX,
        LED_COL_SEVEN,
        LED_COL_ELEVEN,
    };

    if (time > update_time_ms)
    {
        leds->controller->setAllLEDPWM(0);
        leds->controller->setColumn(col_order[step], leds->brightness);

        update_time_ms = time + leds->delay_ms;

        step++;
        if (step >= max_steps)
        {
            step = 0;
        }
    }
}
