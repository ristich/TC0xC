#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
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

enum { bit_err, bit_start, bit_high, bit_low }; // for printf

static struct _b1{
   bool reset;
   int index;
   int bits;
   int mask;
   int bit_enum;                       // 'E', 'S', 'H','L'
}hbits; // for pulse times to bits

static char *bit_titles[4] = { "Err", "Start", "High", "Low"};

static void IRAM_ATTR isr_IR();
static void ir_decode(int pulse);
static QueueHandle_t ir_queue;

static int icount;
int pulse_buffer[PULSE_BUFFER_SIZE];
int pulse_buffer_index = 0;

void ir_tx_task(void *pvParameters);
void ir_rx_task(void *paramaters);

IR_Error ir_init(IR_Object *ir)
{
    if (ir->initialized)
        return IR_SUCCESS;

    IRsend irsend(IR_TX_PIN);
    irsend.begin();
    xTaskCreatePinnedToCore(ir_rx_task, "ir_rx_task", 2048, NULL, tskIDLE_PRIORITY + 2, &ir->rx_task_handle, app_cpu);
    xTaskCreatePinnedToCore(ir_tx_task, "ir_tx_task", 2048, NULL, tskIDLE_PRIORITY + 2, &ir->tx_task_handle, app_cpu);

    ir->initialized = true;

    return IR_SUCCESS;
}

void ir_tx_task(void *pvParameters)
{
  while(1){
    if (xTaskNotifyWaitIndexed(0, 0, 0, 0, 0) == pdTRUE)
    {
      IRsend irsend(IR_TX_PIN);
      uint8_t checksum = ((ESP.getEfuseMac() >> 24) & 0xFF) ^ ((ESP.getEfuseMac() >> 32) & 0xFF) ^ ((ESP.getEfuseMac() >> 40));
      Serial.printf("IR TX: %X\n",((ESP.getEfuseMac() >> 24) | (checksum << 24))); //todo replace with fire phrase
      irsend.sendNEC((ESP.getEfuseMac() >> 24) | (checksum << 24));
    }
    vTaskDelay(10);
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/// \brief task to decode infrared pulse times from ISR queue
void ir_rx_task( void * parameter ){
   float fv=0;
   const TickType_t xDelay = 600 / portTICK_PERIOD_MS;
   int pulse;

   ir_queue = xQueueCreate( 100, sizeof( int ) );

   if(ir_queue == NULL){
      Serial.println("Error creating the ir_queue");
   }

   Serial.print("ir_rx_task...\n");
   // setup ISR to trigger RISING|FALLING
   pinMode(IR_RX_PIN,INPUT);
   attachInterrupt(IR_RX_PIN, isr_IR, CHANGE);
   hbits.reset = true;

   for(;;){
      if( xQueueReceive(ir_queue, &pulse, portMAX_DELAY) ){
              ir_decode(pulse);
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
static void ir_decode(int pulse) {
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
    // Print the received hexadecimal code - todo comment out
    printf("Received NEC code: %08lX\n", received_bits); //todo comment out or enable only in developer mode
    // Reset pulse buffer index
    pulse_buffer_index = 0;
  }

}
