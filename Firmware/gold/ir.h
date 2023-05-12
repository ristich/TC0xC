#pragma once

typedef enum IR_Error
{
    IR_SUCCESS = 0,
} IR_Error;

// audio object to be passed around for audio control
typedef struct IR_Object
{
    bool initialized;         // state of audio task creation
    TaskHandle_t task_handle; // task handle
} IR_Object;

IR_Error ir_init(IR_Object *ir);