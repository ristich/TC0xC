#include "led.h"

static void LED_task(void *pvParameters);
void set_LED_mode(LED_Object *leds);

static void rotate_leds(LED_Object *leds);
static void blink_leds(LED_Object *leds);

TC_IS31FL3731 led_controller = TC_IS31FL3731();

LED_Error LED_init(LED_Object *leds)
{
    if (leds->initialized)
    {
        return LED_SUCCESS;
    }

    leds->update_sem = xSemaphoreCreateBinary();

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
    // leds->mode = LED_MODE_BLINK;
    leds->delay_ms = 500;
    leds->brightness = 10;

    xTaskCreatePinnedToCore(LED_task, "LED_task", 2048, leds, tskIDLE_PRIORITY + 2, NULL, app_cpu);

    leds->initialized = true;

    return LED_SUCCESS;
}

void LED_task(void *pvParameters)
{
    LED_Object *leds = (LED_Object *)pvParameters;
    set_LED_mode(leds);
    static const TickType_t alert_period_ms = 30000; // 30s time between alerts
    static TickType_t next_alert_time_ms = alert_period_ms;

    // signal to cli that leds are set up
    xSemaphoreGive(leds->update_sem);

    // wait for cli to init
    vTaskDelay(500 / portTICK_PERIOD_MS);

    while (1)
    {
        TickType_t task_time_ms = xTaskGetTickCount();

        // check an update
        if (xSemaphoreTake(leds->update_sem, 10))
        {
            set_LED_mode(leds);
            xSemaphoreGive(leds->update_sem);
        }
        
        if (task_time_ms > next_alert_time_ms)
        {
            leds->controller->setAllLEDPWM(0);
            leds->controller->setDisplayMode(Display_Mode_Picture);
            leds->controller->setPictureFrame(0);
            vTaskDelay(500);
            leds->controller->setAllLEDPWM(leds->brightness);
            vTaskDelay(500);
            leds->controller->setAllLEDPWM(0);
            vTaskDelay(500);
            leds->controller->setAllLEDPWM(leds->brightness);
            vTaskDelay(500);
            leds->controller->setAllLEDPWM(0);
            vTaskDelay(500);
            leds->controller->setAllLEDPWM(leds->brightness);
            vTaskDelay(500);

            next_alert_time_ms = task_time_ms + alert_period_ms;
            set_LED_mode(leds);
        }
    }
}

void set_LED_mode(LED_Object *leds)
{
    switch (leds->mode)
    {
    case LED_MODE_OFF:
        leds->controller->setAllLEDPWM(0);
        break;

    case LED_MODE_ROTATE:
        rotate_leds(leds);
        break;
    
    case LED_MODE_BLINK:
        blink_leds(leds);
        break;

    default:
        leds->controller->setAllLEDPWM(0);
        break;
    }
}

static void rotate_leds(LED_Object *leds)
{
    leds->controller->setAutoPlayFrames(6);
    leds->controller->setAutoPlayLoops(0);
    leds->controller->setAutoPlayDelay(leds->delay_ms);
    leds->controller->setAutoPlayStart(1);

    leds->controller->setAllLEDPWM(0, 1);
    leds->controller->setAllLEDPWM(0, 2);
    leds->controller->setAllLEDPWM(0, 3);
    leds->controller->setAllLEDPWM(0, 4);
    leds->controller->setAllLEDPWM(0, 5);
    leds->controller->setAllLEDPWM(0, 6);

    // prepare the show
    leds->controller->setColumn(LED_COL_TWELVE, leds->brightness, 1);
    leds->controller->setColumn(LED_COL_ONE, leds->brightness, 2);
    leds->controller->setColumn(LED_COL_FIVE, leds->brightness, 3);
    leds->controller->setColumn(LED_COL_SIX, leds->brightness, 4);
    leds->controller->setColumn(LED_COL_SEVEN, leds->brightness, 5);
    leds->controller->setColumn(LED_COL_ELEVEN, leds->brightness, 6);

    // showtime!
    leds->controller->setDisplayMode(Display_Mode_Auto_Play);
}

static void blink_leds(LED_Object *leds)
{
    leds->controller->setAutoPlayFrames(2);
    leds->controller->setAutoPlayLoops(0);
    leds->controller->setAutoPlayDelay(leds->delay_ms);
    leds->controller->setAutoPlayStart(1);

    leds->controller->setAllLEDPWM(0, 1);
    leds->controller->setAllLEDPWM(0, 2);

    leds->controller->setAllLEDPWM(leds->brightness, 2);
    
    leds->controller->setDisplayMode(Display_Mode_Auto_Play);
}
