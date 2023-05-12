#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "rtos.h"
#include "ir.h"
#include "hal.h"

void ir_task(void *pvParameters);

IR_Error ir_init(IR_Object *ir)
{
    if (ir->initialized)
        return IR_SUCCESS;

    IRsend irsend(IR_TX_PIN);
    irsend.begin();
    xTaskCreatePinnedToCore(ir_task, "ir_task", 2048, NULL, tskIDLE_PRIORITY + 2, &ir->task_handle, app_cpu);

    ir->initialized = true;

    return IR_SUCCESS;
}

void ir_task(void *pvParameters)
{
  while(1){
    if (xTaskNotifyWaitIndexed(0, 0, 0, 0, 0) == pdTRUE)
    {
      IRsend irsend(IR_TX_PIN);
      uint8_t checksum = ((ESP.getEfuseMac() >> 24) & 0xFF) ^ ((ESP.getEfuseMac() >> 32) & 0xFF) ^ ((ESP.getEfuseMac() >> 40));
      Serial.printf("IR TX: %X",((ESP.getEfuseMac() >> 24) | (checksum << 24)));
      irsend.sendNEC((ESP.getEfuseMac() >> 24) | (checksum << 24));
    }
    vTaskDelay(10);
  }
}
