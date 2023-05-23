#include "ir.h"
#pragma once

#include "led.h"
#define CLI_ARG_COUNT_MAX 7
#define BUFFER_SIZE 40

typedef enum CLI_Error
{
    CLI_SUCCESS = 0,
} CLI_Error;

// cli object to be passed around for cli access
typedef struct CLI_Object
{
    LED_Object *leds;            // pointer to led object
    HardwareSerial *serial;      // pointer to serial interface
    bool dev_mode;               // dev mode setting
    char rx_buffer[BUFFER_SIZE]; // received serial input
    uint32_t ret_addr;           // return address for simulated buffer overflow
    uint8_t rx_len;              // current input length
    IR_Object *ir;               // pointer to IR object
} CLI_Object;

// cli callback function template
typedef void (*cmd_cb_t)(CLI_Object *cli, uint8_t nargs, char **args);

// command template
typedef struct CLI_Command
{
    const char *cmd_name; // command name
    const char *cmd_desc; // command description
    cmd_cb_t cmd_cb;      // callback function
    bool cmd_hidden;      // hidden from help menu
} CLI_Command;

CLI_Error CLI_init(CLI_Object *cli, LED_Object *leds, IR_Object *ir);