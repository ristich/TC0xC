#include "IRCClient.h"
#pragma once
#include <Arduino.h>

typedef enum IR_Error
{
    IR_SUCCESS = 0,
} IR_Error;

// audio object to be passed around for audio control
typedef struct IR_Object
{
    bool initialized;         // state of audio task creation
    TaskHandle_t tx_task_handle; // task handle
    TaskHandle_t rx_task_handle;
    uint32_t last_code;
    IRCClient* irc;
} IR_Object;

IR_Error ir_init(IR_Object *ir, IRCClient *irc);
