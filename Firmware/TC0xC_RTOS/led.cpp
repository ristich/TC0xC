#include "led.h"

static void LED_task(void *pvParameters);
void set_LED_mode(LED_Object *leds);
void init_timer(LED_Object *leds);
void ARDUINO_ISR_ATTR onTimer();

// LED modes
static void rotate_leds(LED_Object *leds);
static void blink_leds(LED_Object *leds);

// constants
const uint32_t Alarm_Interval_Long = 70000;  // 7s
const uint32_t Alarm_Interval_Short = 50000; // 5s
const uint8_t message_brightness = 100;
const uint16_t message_delay_ms = 550;

TC_IS31FL3731 led_controller = TC_IS31FL3731();
volatile SemaphoreHandle_t timerSemaphore;

LED_Error LED_init(LED_Object *leds)
{
    if (leds->initialized)
    {
        return LED_SUCCESS;
    }

    leds->update_sem = xSemaphoreCreateBinary();

    init_timer(leds);

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

void init_timer(LED_Object *leds)
{
    // Create semaphore to inform us when the timer has fired
    timerSemaphore = xSemaphoreCreateBinary();

    // use first timer with 10kHz clock
    leds->message_timer = timerBegin(0, 8000, true);
    timerAttachInterrupt(leds->message_timer, &onTimer, true);

    // Set alarm to trigger after 10s
    timerAlarmWrite(leds->message_timer, Alarm_Interval_Long, true);

    // Start an alarm
    timerAlarmEnable(leds->message_timer);
}

void LED_task(void *pvParameters)
{
    LED_Object *leds = (LED_Object *)pvParameters;
    set_LED_mode(leds);
    static bool alarm_is_long = true;

    // signal to cli that leds are set up
    xSemaphoreGive(leds->update_sem);

    // wait for cli to init
    vTaskDelay(500 / portTICK_PERIOD_MS);

    while (1)
    {
        // check an update
        if (xSemaphoreTake(leds->update_sem, 0) == pdTRUE)
        {
            set_LED_mode(leds);
            xSemaphoreGive(leds->update_sem);
        }

        if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
        {
            alarm_is_long = !alarm_is_long;

            leds->controller->setAllLEDPWM(0);
            leds->controller->setDisplayMode(Display_Mode_Picture);
            leds->controller->setPictureFrame(0);

            if (alarm_is_long)
            {
                timerAlarmWrite(leds->message_timer, Alarm_Interval_Long, true);
                char message[] = "helloworld";
                leds->controller->setBadgeMessage(message, (sizeof(message) - 1), message_brightness, message_delay_ms);
            }
            else
            {
                timerAlarmWrite(leds->message_timer, Alarm_Interval_Short, true);
                char message[] = "foobar";
                leds->controller->setBadgeMessage(message, (sizeof(message) - 1), message_brightness, message_delay_ms);
            }

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

void ARDUINO_ISR_ATTR onTimer()
{
    // use semaphore to alert loop of timer trigger
    xSemaphoreGiveFromISR(timerSemaphore, NULL);
}
