#include "HardwareSerial.h"
#include "IRCClient.h"
#include <cmath>
#include <stdio.h>
#include <sys/_stdint.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <EEPROM.h>
#include "rtos.h"
#include "ir.h"
#include "hal.h"

enum { //NEC IR protocol spec
    eStart = 9000,     // Leader pulse duration (around 9000us)
    eSpace = 4500,     // Space pulse duration (around 2400us)
    eBit0 = 560,       // '0' bit pulse duration (around 560us)
    eBit1 = 560,       // '1' bit pulse duration (around 560us)
    eBit1Space = 1690
};

static void IRAM_ATTR isr_IR();
static void ir_decode(int pulse, uint32_t* decoded, IRCClient* irc);
static QueueHandle_t ir_queue;
bool fireCode(uint32_t code);

static int icount;
int pulse_buffer[PULSE_BUFFER_SIZE];
int pulse_buffer_index = 0;

void ir_tx_task(void *pvParameters);
void ir_rx_task(void *paramaters);

IR_Error ir_init(IR_Object *ir, IRCClient *irc)
{
    if (ir->initialized)
        return IR_SUCCESS;

    IRsend irsend(IR_TX_PIN);
    irsend.begin();
    xTaskCreatePinnedToCore(ir_rx_task, "ir_rx_task", 2048, ir, tskIDLE_PRIORITY + 2, &ir->rx_task_handle, app_cpu);
    xTaskCreatePinnedToCore(ir_tx_task, "ir_tx_task", 2048, NULL, tskIDLE_PRIORITY + 2, &ir->tx_task_handle, app_cpu);

    ir->initialized = true;
    ir->last_code = 0x0;
    ir->irc = irc;
    return IR_SUCCESS;
}

void ir_tx_task(void *pvParameters)
{
  while(1){
    if (xTaskNotifyWaitIndexed(0, 0, 0, 0, 0) == pdTRUE)
    {
      IRsend irsend(IR_TX_PIN);
      uint8_t checksum = ((ESP.getEfuseMac() >> 24) & 0xFF) ^ ((ESP.getEfuseMac() >> 32) & 0xFF) ^ ((ESP.getEfuseMac() >> 40));
      //Serial.printf("IR TX: %X\n",((ESP.getEfuseMac() >> 24) | (checksum << 24))); //todo replace with fire phrase
      //irsend.sendNEC((ESP.getEfuseMac() >> 24) | (checksum << 24));
      irsend.sendNEC(0x464C4147);
    }
    vTaskDelay(10);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// \brief task to decode infrared pulse times from ISR queue
void ir_rx_task( void * parameter ){

   IR_Object *ir = (IR_Object *)parameter;
   float fv=0;
   const TickType_t xDelay = 600 / portTICK_PERIOD_MS;
   int pulse;

   ir_queue = xQueueCreate( 100, sizeof( int ) );

   if(ir_queue == NULL){
      Serial.println("Error creating the ir_queue");
   }

   Serial.print("Infrared Lock-On Acheived.  Specturm Scanning:\n");
   // setup ISR to trigger RISING|FALLING
   pinMode(IR_RX_PIN,INPUT);
   attachInterrupt(IR_RX_PIN, isr_IR, CHANGE);

   for(;;){
      if( xQueueReceive(ir_queue, &pulse, portMAX_DELAY) ){
              ir_decode(pulse,&(ir->last_code),ir->irc);
      }
   }
   // we'll never get here
   printf("Ending task 1\n");
   vTaskDelete( NULL );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// \brief ISR edge trigged by 38khz IR signal
static void IRAM_ATTR isr_IR(){
   static uint32_t t0, t1;
   int intval;
   //measure the pulse duration
   t1 = micros();
   intval = t1 - t0;
   t0 = t1;
   //drop it in the queue
   xQueueSendFromISR( ir_queue, &intval, NULL);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// \brief decde against an NEC-like IR pulse sequence
static void ir_decode(int pulse, uint32_t *decoded, IRCClient* irc) {
   if (pulse > (eStart * (1 + TOLERANCE)) || pulse < (eBit0 *(1 - TOLERANCE))) {
    // Pulse duration we do not expect, ignore and return
    //printf("Ignoring pulse duration: %d\n", pulse);
    return;
  }

  //printf("Received pulse: %d, Buffer index: %d\n", pulse, pulse_buffer_index);

    // Check if pulse duration is within the tolerance of the start pulse
  if (pulse > eStart * (1 - TOLERANCE) && pulse < eStart * (1 + TOLERANCE)) {
    // Place the pulse duration in the buffer at position 0
    pulse_buffer[0] = pulse;
    pulse_buffer_index = 1;
    return;
  }

  // Check if pulse duration is within the tolerance of the space pulse
  if (pulse > eSpace * (1 - TOLERANCE) && pulse < eSpace * (1 + TOLERANCE)) {
    // Place the pulse duration in the buffer at position 1
    pulse_buffer[1] = pulse;
    pulse_buffer_index = 2;
    return;
  }

  // Store the pulse duration in the buffer
  pulse_buffer[pulse_buffer_index++] = pulse;

  // Check if pulse buffer is full, if so decode
  if (pulse_buffer_index >= PULSE_BUFFER_SIZE) {
    // Start decoding the bits
    uint32_t received_bits = 0;
    for (int i = 2; i < PULSE_BUFFER_SIZE; i = i+2) {
      // Check two pulses at a time to determine bits.
      // Check if pulse duration matches the '0' or '1' bit
      if (pulse_buffer[i+1] > eBit0 * (1 - TOLERANCE) && pulse_buffer[i+1] < eBit0 * (1 + TOLERANCE)) {
        // '0' bit
        received_bits <<= 1;
      } else if (pulse_buffer[i+1] > eBit1Space * (1 - TOLERANCE) && pulse_buffer[i+1] < eBit1Space * (1 + TOLERANCE)) {
        // '1' bit
        received_bits = (received_bits << 1) | 0x01;
      } else {
        // Invalid pulse duration, restart buffer filling
        pulse_buffer_index = 0;
       // printf("Invalid pulse duration: Got to bit %d\n", i);
        return;
      }
    }
    
    if (EEPROM.readByte(EEPROM_ADDR_DEV_MODE) == 1){
       printf("Received NEC code: %08lX\n", received_bits); //enabled only in developer mode

    }
    
    if (received_bits && fireCode(received_bits))
    { // ignore null results and see if its from a badge
        String msg;
        String enemy = String((received_bits & 0x00FFFFFF),HEX);
        enemy.toUpperCase();
        //printf("damage control\n"); 
        msg = "PRIVMSG #battles :received 1 damage from TC";
        msg += enemy;
        //Serial.println(msg);
        irc->sendRaw(msg);
    }
    else if (received_bits == 0x464C4147){
      String msg;
      msg = "received_bits";
      Serial.println(msg);
    }
          
    // Reset pulse buffer index
    *(decoded) = received_bits;
    pulse_buffer_index = 0;
  }

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// \brief evaluates codes to see if TCBadge would have generate them (mostly)
bool fireCode(uint32_t input) { // calculates the checksum to see if this is generated from another badge
  uint8_t checksum = ((input >> 16) & 0xFF) ^ ((input >> 8) & 0xFF) ^ ((input & 0xFF));
  uint8_t expectedChecksum = (input >> 24) & 0xFF;
  return (checksum == expectedChecksum);
}
