#include <Arduino.h>
#include <EEPROM.h>
#include "rtos.h"
#include "led.h"
#include "hal.h"

static void LED_task(void *pvParameters);
void set_LED_mode(LED_Object *leds);
void init_timer(LED_Object *leds);
void ARDUINO_ISR_ATTR onTimer();

// LED modes
static void boot_sequence(LED_Object *leds);
static void clear_leds(LED_Object *leds);
static void rings_leds(LED_Object *leds, bool outward);
static void rotate_leds(LED_Object *leds, bool clockwise);
static void blink_leds(LED_Object *leds);
static void party_sequence(LED_Object *leds);

// constants
const uint32_t Alarm_Interval_Long = 600000;  // 5 min
const uint32_t Alarm_Interval_Short = 30000; // 2 min
const uint8_t Message_Brightness = 200;
const uint16_t Message_Delay_ms = 150;
const uint8_t MAX_MESSAGE_LEN = 50;
const uint8_t TOTAL_MESSAGES = 5;

char Messages[TOTAL_MESSAGES][MAX_MESSAGE_LEN] = {"up up down down left right left right b a start",
                                                  "a fork and a knife"
                                                  "a bottle and a cork",
                                                  "foobar",
                                                  "abcxzy"};

TC_IS31FL3731 led_controller = TC_IS31FL3731();
volatile SemaphoreHandle_t timerSemaphore;

LED_Error LED_init(LED_Object *leds)
{
    if (leds->initialized)
    {
        return LED_SUCCESS;
    }

    init_timer(leds);

    // init pins for led driver
    pinMode(I2C_SDA_PIN, OUTPUT);
    pinMode(I2C_SCL_PIN, OUTPUT);
    pinMode(LED_SDB_PIN, OUTPUT);
    digitalWrite(LED_SDB_PIN, HIGH);

    // init led driver
    led_controller.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    leds->controller = &led_controller;

    // get saved settings
    leds->mode = (led_mode_t)EEPROM.readByte(EEPROM_ADDR_LED_MODE);
    leds->delay_ms = EEPROM.readUShort(EEPROM_ADDR_LED_DELAY);
    if (leds->delay_ms < 11)
        leds->delay_ms = 11;
    leds->brightness = EEPROM.readByte(EEPROM_ADDR_LED_BRIGHT);

    bool update = false;
    if (leds->mode >= LED_MODE_TOTAL)
    {
        leds->mode = Led_Mode_Default;
        EEPROM.writeByte(EEPROM_ADDR_LED_MODE, leds->mode);
        update = true;
    }
    if (leds->delay_ms > LED_MAX_DELAY_ms)
    {
        leds->delay_ms = Led_Delay_Default;
        EEPROM.writeUShort(EEPROM_ADDR_LED_DELAY, leds->delay_ms);
        update = true;
    }
    if (leds->brightness > LED_MAX_BRIGHTNES)
    {
        leds->brightness = Led_Brightness_Default;
        EEPROM.writeByte(EEPROM_ADDR_LED_BRIGHT, leds->brightness);
        update = true;
    }
    if (update)
    {
        EEPROM.commit();
    }

    xTaskCreatePinnedToCore(LED_task, "LED_task", 2048, leds, tskIDLE_PRIORITY + 2, &leds->task_handle, app_cpu);

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

    // Set alarm to trigger after interval to display message
    timerAlarmWrite(leds->message_timer, Alarm_Interval_Long, true);
    timerAlarmEnable(leds->message_timer);
}

void LED_task(void *pvParameters)
{
    LED_Object *leds = (LED_Object *)pvParameters;
    //boot_sequence(leds);
    set_LED_mode(leds);
    static bool alarm_is_long = true;
    static uint8_t message_index = 0;
    uint32_t led_event = 0;
    uint32_t *pLed_event = &led_event;

    // wait for cli to init
    vTaskDelay(500 / portTICK_PERIOD_MS);

    while (1)
    {
        // check for timed message
        if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
        {
            if (alarm_is_long)
                timerAlarmWrite(leds->message_timer, Alarm_Interval_Short, true);
            else
                timerAlarmWrite(leds->message_timer, Alarm_Interval_Long, true);

            leds->controller->setAllLEDPWM(0);
            leds->controller->setDisplayMode(Display_Mode_Picture);
            leds->controller->setPictureFrame(0);
            leds->controller->setBadgeMessage(Messages[message_index], MAX_MESSAGE_LEN, Message_Brightness, Message_Delay_ms);

            alarm_is_long = !alarm_is_long;
            message_index++;
            if (message_index >= TOTAL_MESSAGES)
                message_index = 0;

            set_LED_mode(leds);
        }

        if (xTaskNotifyWaitIndexed(0, 0, 0, pLed_event, 0) == pdTRUE)
        {
            // don't need to call this right now, since set_LED_mode gets called any
            // time there is an alert currently
            // if (led_event == LED_UPDATE)
            // {
            //     set_LED_mode(leds);
            // }
            if (led_event == LED_BUTTON_PRESS)
            {
                clear_leds(leds);
                leds->controller->setAllLEDPWM(100);
                vTaskDelay(200);
            }
            xTaskNotifyStateClearIndexed(leds->task_handle, 0);

            set_LED_mode(leds);
        }
        vTaskDelay(10);
    }
}

void set_LED_mode(LED_Object *leds)
{
    switch (leds->mode)
    {
    case LED_MODE_OFF:
        clear_leds(leds);
        break;

    case LED_MODE_ROTATE_CW:
        rotate_leds(leds, true);
        break;

    case LED_MODE_ROTATE_CCW:
        rotate_leds(leds, false);
        break;

    case LED_MODE_BLINK:
        blink_leds(leds);
        break;

    case LED_MODE_RINGS_IN:
        rings_leds(leds, false);
        break;

    case LED_MODE_RINGS_OUT:
        rings_leds(leds, true);
        break;

    default:
        break;
    }
}

  static void boot_sequence(LED_Object *leds)
  {
      //Serial.println("Boot Anim..");
      leds->controller->clear();
      uint8_t i, j;
      uint32_t led_rings[4] = {LED_RING_INNER, LED_RING_MID_INNER, LED_RING_MID_OUTER, LED_RING_OUTER};
      const TickType_t blink_delay_ms = 400;
      const TickType_t hold_delay_ms = 1000;
      for (i = 0; i < 4; i++)
      {
          for (j = 0; j < 3; j++)
          {
              leds->controller->setBadgeLEDs(led_rings[i], leds->brightness);
              vTaskDelay(blink_delay_ms);
              leds->controller->clear();
              vTaskDelay(blink_delay_ms);
          }
      }
      j = 0;
      for (i = 512; i > 0; i -= 32)
      {
          leds->controller->clear();
          leds->controller->setColumn((led_col_t)j, leds->brightness, 0);
          vTaskDelay(i);
          j = (j + 1) % LED_COL_COUNT;
      }
      leds->controller->setAllLEDPWM(leds->brightness);
      vTaskDelay(hold_delay_ms);
      leds->controller->clear();
      vTaskDelay(blink_delay_ms);
      char boot_message[] = "welcome to thotcon";
      leds->controller->setBadgeMessage(boot_message, sizeof(boot_message) / sizeof(char), Message_Brightness, Message_Delay_ms);
      leds->controller->clear();
  }

static void clear_leds(LED_Object *leds)
{
    leds->controller->setAllLEDPWM(0, 0);
    leds->controller->setDisplayMode(Display_Mode_Picture);
    leds->controller->setPictureFrame(0);
}

static void rings_leds(LED_Object *leds, bool outward)
{
    leds->brightness = 200;
    leds->controller->setAutoPlayFrames(4);
    leds->controller->setAutoPlayLoops(0);
    leds->controller->setAutoPlayDelay(leds->delay_ms);
    leds->controller->setAutoPlayStart(1);

    leds->controller->setAllLEDPWM(0, 1);
    leds->controller->setAllLEDPWM(0, 2);
    leds->controller->setAllLEDPWM(0, 3);
    leds->controller->setAllLEDPWM(0, 4);

    if (outward)
    {
        leds->controller->setBadgeLEDs(LED_RING_INNER, leds->brightness, 1);
        leds->controller->setBadgeLEDs(LED_RING_MID_INNER, leds->brightness, 2);
        leds->controller->setBadgeLEDs(LED_RING_MID_OUTER, leds->brightness, 3);
        leds->controller->setBadgeLEDs(LED_RING_OUTER, leds->brightness, 4);
    }
    else
    {
        leds->controller->setBadgeLEDs(LED_RING_OUTER, leds->brightness, 1);
        leds->controller->setBadgeLEDs(LED_RING_MID_OUTER, leds->brightness, 2);
        leds->controller->setBadgeLEDs(LED_RING_MID_INNER, leds->brightness, 3);
        leds->controller->setBadgeLEDs(LED_RING_INNER, leds->brightness, 4);
    }

    // showtime!
    leds->controller->setDisplayMode(Display_Mode_Auto_Play);
}

static void party_sequence(LED_Object *leds)
{
    leds->controller->clear();
    leds->controller->setAutoPlayFrames(20);
    leds->controller->setAutoPlayLoops(0);
    leds->controller->setAutoPlayDelay(leds->delay_ms);
    leds->controller->setAutoPlayStart(1);

    leds->controller->setBadgeLEDs(LED_RING_INNER, leds->brightness, 1);
    leds->controller->setBadgeLEDs(LED_RING_MID_INNER, leds->brightness, 2);
    leds->controller->setBadgeLEDs(LED_RING_MID_OUTER, leds->brightness, 3);
    leds->controller->setBadgeLEDs(LED_RING_OUTER, leds->brightness, 4);

    leds->controller->setColumn(LED_COL_TWELVE, leds->brightness, 5);
    leds->controller->setColumn(LED_COL_ONE, leds->brightness, 6);
    leds->controller->setColumn(LED_COL_FIVE, leds->brightness, 7);
    leds->controller->setColumn(LED_COL_SIX, leds->brightness, 8);
    leds->controller->setColumn(LED_COL_SEVEN, leds->brightness, 9);
    leds->controller->setColumn(LED_COL_ELEVEN, leds->brightness, 10);

     leds->controller->setColumn(LED_COL_TWELVE, leds->brightness, 11);
        leds->controller->setColumn(LED_COL_ELEVEN, leds->brightness, 12);
        leds->controller->setColumn(LED_COL_SEVEN, leds->brightness, 13);
        leds->controller->setColumn(LED_COL_SIX, leds->brightness, 14);
        leds->controller->setColumn(LED_COL_FIVE, leds->brightness, 15);
        leds->controller->setColumn(LED_COL_ONE, leds->brightness, 16);

        leds->controller->setBadgeLEDs(LED_RING_OUTER, leds->brightness, 17);
        leds->controller->setBadgeLEDs(LED_RING_MID_OUTER, leds->brightness, 18);
        leds->controller->setBadgeLEDs(LED_RING_MID_INNER, leds->brightness, 19);
        leds->controller->setBadgeLEDs(LED_RING_INNER, leds->brightness, 20);

        leds->controller->setAllLEDPWM(leds->brightness);

      leds->controller->setDisplayMode(Display_Mode_Auto_Play);
      vTaskDelay(10000);




  }

static void rotate_leds(LED_Object *leds, bool clockwise)
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
    if (clockwise)
    {
        leds->controller->setColumn(LED_COL_TWELVE, leds->brightness, 1);
        leds->controller->setColumn(LED_COL_ONE, leds->brightness, 2);
        leds->controller->setColumn(LED_COL_FIVE, leds->brightness, 3);
        leds->controller->setColumn(LED_COL_SIX, leds->brightness, 4);
        leds->controller->setColumn(LED_COL_SEVEN, leds->brightness, 5);
        leds->controller->setColumn(LED_COL_ELEVEN, leds->brightness, 6);
    }
    else
    {
        leds->controller->setColumn(LED_COL_TWELVE, leds->brightness, 1);
        leds->controller->setColumn(LED_COL_ELEVEN, leds->brightness, 2);
        leds->controller->setColumn(LED_COL_SEVEN, leds->brightness, 3);
        leds->controller->setColumn(LED_COL_SIX, leds->brightness, 4);
        leds->controller->setColumn(LED_COL_FIVE, leds->brightness, 5);
        leds->controller->setColumn(LED_COL_ONE, leds->brightness, 6);
    }

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
