#include "Printable.h"
#include "ir.h"
#include "cli.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "rtos.h"
#include "hal.h"

static void CLI_task(void *pvParameters);
static bool process_byte(CLI_Object *cli, char rxByte);
static void CLI_handle_command(CLI_Object *cli, const CLI_Command *cmd_list, uint8_t cmd_count);
char buff_overflow(uint32_t return_addr);

// command callback funcations
static void CLI_Test(CLI_Object *cli, uint8_t nargs, char **args);
static void CLI_Help(CLI_Object *cli, uint8_t nargs, char **args);
static void CLI_Flag(CLI_Object *cli, uint8_t nargs, char **args);
static void CLI_Reset(CLI_Object *cli, uint8_t nargs, char **args);
static void CLI_IRcode(CLI_Object *cli, uint8_t nargs, char **args);

static void LED_Brightness(CLI_Object *cli, uint8_t nargs, char **args);
static void LED_Delay(CLI_Object *cli, uint8_t nargs, char **args);
static void LED_Pattern(CLI_Object *cli, uint8_t nargs, char **args);

// list of commands, descriptions, and callback functions
static const CLI_Command CMD_List[] = {
    {.cmd_name = "test", .cmd_desc = "hello world", .cmd_cb = CLI_Test, .cmd_hidden = true},
    {.cmd_name = "help", .cmd_desc = "list commands", .cmd_cb = CLI_Help, .cmd_hidden = false},
    {.cmd_name = "brightness", .cmd_desc = "set led brightness [0 - 128]", .cmd_cb = LED_Brightness, .cmd_hidden = false},
    {.cmd_name = "delay", .cmd_desc = "set led delay (in ms) [0 - 693]", .cmd_cb = LED_Delay, .cmd_hidden = false},
    {.cmd_name = "pattern", .cmd_desc = "set led pattern [0 - ", .cmd_cb = LED_Pattern, .cmd_hidden = false},
    {.cmd_name = "reset", .cmd_desc = "reset settings", .cmd_cb = CLI_Reset, .cmd_hidden = false},
    {.cmd_name = "flag", .cmd_desc = "print flag", .cmd_cb = CLI_Flag, .cmd_hidden = true},
    {.cmd_name = "ircode", .cmd_desc = "print last IR code recieved", .cmd_cb = CLI_IRcode, .cmd_hidden = true},
};
// number of commands
static const uint8_t CMD_Count = sizeof(CMD_List) / sizeof(CLI_Command);

/**
 * @brief initialize and start command-line interface task
 *
 * @param serial pointer to a running serial interface
 * @return CLI_Error
 */
CLI_Error CLI_init(CLI_Object *cli, LED_Object *leds, IR_Object *ir)
{
    static bool initialized = false;
    if (initialized)
    {
        return CLI_SUCCESS;
    }

    Serial.begin(115200);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // create CLI object to pass around for serial and buffer acccess
    cli->leds = leds,
    cli->serial = &Serial,
    cli->rx_len = 0,
    cli->ir = ir,
    xTaskCreatePinnedToCore(CLI_task, "CLI_task", 2048, cli, tskIDLE_PRIORITY + 3, NULL, app_cpu);

    initialized = true;

    return CLI_SUCCESS;
}

/**
 * @brief funning task that handles CLI functionality
 *
 * @note this function should never return
 *
 * @param pvParameters pointer to CLI_Object containing CLI and application data
 */
static void CLI_task(void *pvParameters)
{
    CLI_Object *cli = (CLI_Object *)pvParameters;
    cli->serial->println("\nWelcome to CLI");
    
    cli->serial->println("uart flag: flag{welcome_to_the_8bit_world}");
    if (cli->dev_mode)
    {
        cli->serial->print("Developer Mode: ");
        cli->serial->println(cli->dev_mode);
        
        cli->serial->print("LED Pattern: ");
        cli->serial->println(cli->leds->mode);
        cli->serial->print("LED Brightness: ");
        cli->serial->println(cli->leds->brightness);
        cli->serial->print("LED Delay: ");
        cli->serial->print(cli->leds->delay_ms);
        cli->serial->println(" ms");
        
        cli->serial->print("CMD count: ");
        cli->serial->println(CMD_Count);
    }

    while (1)
    {
        while (cli->serial->available())
        {
            char r = cli->serial->read();
            if (process_byte(cli, r))
            {
                // command received
                CLI_handle_command(cli, CMD_List, CMD_Count);

                // clear command buffer
                cli->rx_len = 0;
                cli->rx_buffer[0] = 0;
            }
        }
        vTaskDelay(100);
    }
}

/**
 * @brief process byte received by serial interface
 *
 * @param cli pointer to cli object
 * @param rxByte serial byte received
 * @return true complete command was received
 * @return false buffer is still accumulating
 */
static bool process_byte(CLI_Object *cli, char rxByte)
{
    // if ((rxByte == '\r') || (rxByte == '\n'))
    if (rxByte == '\n')
    {
        // command entered
        if (cli->rx_len > BUFFER_SIZE)
        {
            cli->rx_buffer[BUFFER_SIZE - 1] = 0;
        }
        else
        {
            cli->rx_buffer[cli->rx_len] = 0;
        }
        cli->serial->print(rxByte);
        cli->rx_len = 0;
        return true;
    }
    if ((rxByte == 0x7f) || (rxByte == 0x08))
    {
        // backspace or delete
        if (cli->rx_len > 0)
        {
            cli->rx_len--;
            if (cli->rx_len < BUFFER_SIZE)
            {
                cli->rx_buffer[cli->rx_len] = 0;
            }
        }
        cli->serial->print(rxByte);
    }
    else
    {
        cli->rx_buffer[cli->rx_len] = rxByte;
        cli->rx_len++;
        cli->serial->print(rxByte);
    }
    return false;
}

/**
 * @brief validate and handle command
 *
 * @param cli pointer to CLI object
 * @param cmd_list array of CLI commands
 * @param cmd_count number of CLI commands
 */
static void CLI_handle_command(CLI_Object *cli, const CLI_Command *cmd_list, uint8_t cmd_count)
{
    bool command_found = false;
    uint8_t nargs = 0;
    char *pch;
    char *args[CLI_ARG_COUNT_MAX];

    // parse arguments
    args[nargs] = cli->rx_buffer;
    pch = strpbrk(cli->rx_buffer, " ");
    nargs++;
    if (pch)
    {
        *pch = '\0';
    }

    for (int i = 0; i < cmd_count; i++)
    {
        if (strcmp((char *)cli->rx_buffer, cmd_list[i].cmd_name) == 0)
        {
            command_found = true;

            if (strcmp((char *)cli->rx_buffer, "flag") == 0)
            {
                cli->serial->println("ERR: Permission denied!");
                break;
            }

            while (pch != NULL)
            {
                args[nargs] = pch + 1;
                *pch = '\0';
                pch = strpbrk(cli->rx_buffer, " ");
                nargs++;
                if (nargs >= CLI_ARG_COUNT_MAX)
                {
                    break;
                }
            }

            if (cmd_list[i].cmd_cb != NULL)
            {
                cmd_list[i].cmd_cb(cli, nargs - 1, args);
            }
        }
    }

    if (command_found == false)
    {
        if (cli->ret_addr)
        {
            cmd_cb_t buff_overflow_cb = (cmd_cb_t)cli->ret_addr;
            buff_overflow_cb(cli, 1, NULL);
            cli->ret_addr = 0;
        }

        cli->serial->print("ERR: unknown command '");
        cli->serial->print(cli->rx_buffer);
        cli->serial->println("'");
    }

    return;
}

/**
 * @brief test CLI command callback
 *
 * @param cli pointer to CLI object
 * @param nargs number of arguments
 * @param args array of arguments
 */
static void CLI_Test(CLI_Object *cli, uint8_t nargs, char **args)
{
    cli->serial->println("CLI test function");
}

/**
 * @brief help CLI command callback
 *
 * @param cli pointer to CLI object
 * @param nargs number of arguments
 * @param args array of arguments
 */
static void CLI_Help(CLI_Object *cli, uint8_t nargs, char **args)
{
    cli->serial->println("Available CLI Commands");
    for (int i = 0; i < CMD_Count; i++)
    {
        if (cli->dev_mode)
        {
            // list item start address
            cli->serial->print("0x");
            cli->serial->print((uint32_t)CMD_List[i].cmd_name, HEX);
            cli->serial->print(": ");

            // name
            cli->serial->print("cmd: ");
            cli->serial->print(CMD_List[i].cmd_name);
            cli->serial->print(" - ");

            // description
            cli->serial->print("desc: ");
            if (strcmp("pattern", CMD_List[i].cmd_name) == 0)
            {
                cli->serial->print(CMD_List[i].cmd_desc);
                cli->serial->print(LED_MODE_TOTAL);
                cli->serial->print("]");
            }
            else
            {
                cli->serial->print(CMD_List[i].cmd_desc);
            }
            cli->serial->print(" - ");

            // cb
            cli->serial->print("cb: ");
            cli->serial->print("0x");
            cli->serial->print((uint32_t)CMD_List[i].cmd_cb, HEX);
            cli->serial->print(" - ");

            // hide
            cli->serial->print("hidden: ");
            cli->serial->println(CMD_List[i].cmd_hidden);
        }
        else if (CMD_List[i].cmd_hidden == false)
        {
            char name[20];
            snprintf(name, sizeof(name), "%-15s - ", CMD_List[i].cmd_name);
            cli->serial->print(name);
            if (strcmp("pattern", CMD_List[i].cmd_name) == 0)
            {
                cli->serial->print(CMD_List[i].cmd_desc);
                cli->serial->print(LED_MODE_TOTAL);
                cli->serial->println("]");
            }
            else
            {
                cli->serial->println(CMD_List[i].cmd_desc);
            }
        }
    }
}

/**
 * @brief Hidden CLI command to print flag
 *
 * @param cli pointer to CLI object
 * @param nargs number of arguments
 * @param args array of arguments
 */
static void CLI_Flag(CLI_Object *cli, uint8_t nargs, char **args)
{
    const char* flag = (char*)(0x95a67dc088a865d3 ^ 0xFeca1ca7Feca1ca7);
    cli->serial->printf("buff overflow: %s\n",flag);
}

static void CLI_Reset(CLI_Object *cli, uint8_t nargs, char **args)
{
    cli->leds->mode = Led_Mode_Default;
    cli->leds->brightness = Led_Brightness_Default;
    cli->leds->delay_ms = Led_Delay_Default;
    cli->dev_mode = 0;

    EEPROM.writeByte(EEPROM_ADDR_LED_MODE, Led_Mode_Default);
    EEPROM.writeByte(EEPROM_ADDR_LED_BRIGHT, Led_Brightness_Default);
    EEPROM.writeUShort(EEPROM_ADDR_LED_DELAY, Led_Delay_Default);
    EEPROM.writeByte(EEPROM_ADDR_DEV_MODE, 0);
    EEPROM.commit();

    cli->serial->println("Settings reset complete");
}

static void LED_Brightness(CLI_Object *cli, uint8_t nargs, char **args)
{
    if (nargs != 1)
    {
        cli->serial->print("ERR: expected 1 argument, received ");
        cli->serial->println(nargs);
        return;
    }

    if (cli->leds->initialized == false)
    {
        cli->serial->println("ERR: led driver uninitialized");
        return;
    }

    uint32_t brightness = strtoul(args[1], NULL, 10);
    if (brightness > LED_MAX_BRIGHTNES)
    {
        brightness = LED_MAX_BRIGHTNES;
    }
    cli->leds->brightness = (uint8_t)(brightness & 0xFF);
    EEPROM.writeByte(EEPROM_ADDR_LED_BRIGHT, brightness);
    EEPROM.commit();

    // signal to led driver there's an update
    xTaskNotifyIndexed(cli->leds->task_handle, 0, LED_UPDATE, eSetValueWithOverwrite);

    cli->serial->print("LED brightness set to ");
    cli->serial->println(brightness);
}

static void LED_Delay(CLI_Object *cli, uint8_t nargs, char **args)
{
    if (nargs != 1)
    {
        cli->serial->print("ERR: expected 1 argument, received ");
        cli->serial->println(nargs);
        return;
    }

    if (cli->leds->initialized == false)
    {
        cli->serial->println("ERR: led driver uninitialized");
        return;
    }

    // make sure delay isn't above driver's max rating
    uint32_t delay = strtoul(args[1], NULL, 10);
    if (delay > LED_MAX_DELAY_ms)
    {
        delay = LED_MAX_DELAY_ms;
    }

    // delay less than 11ms doesn't do anything
    if (delay < 11)
        cli->leds->delay_ms = 11;
    else
        cli->leds->delay_ms = (uint16_t)(delay & 0xFFFF);
    EEPROM.writeUShort(EEPROM_ADDR_LED_DELAY, delay);
    EEPROM.commit();

    // signal to led driver there's an update
    xTaskNotifyIndexed(cli->leds->task_handle, 0, LED_UPDATE, eSetValueWithOverwrite);

    cli->serial->print("LED delay set to ");
    cli->serial->print(delay);
    cli->serial->println("ms");
}

static void LED_Pattern(CLI_Object *cli, uint8_t nargs, char **args)
{
    if (nargs != 1)
    {
        cli->serial->print("ERR: expected 1 argument, received ");
        cli->serial->println(nargs);
        return;
    }

    if (cli->leds->initialized == false)
    {
        cli->serial->println("ERR: led driver uninitialized");
        return;
    }

    uint32_t pattern = strtoul(args[1], NULL, 10);
    if (pattern >= LED_MODE_TOTAL)
    {
        cli->serial->print("ERR: unknown pattern, max pattern number is ");
        cli->serial->println(LED_MODE_TOTAL);
        return;
    }
    cli->leds->mode = (led_mode_t)(pattern & 0xFF);
    EEPROM.writeByte(EEPROM_ADDR_LED_MODE, pattern);
    EEPROM.commit();

    // signal to led driver there's an update
    xTaskNotifyIndexed(cli->leds->task_handle, 0, LED_UPDATE, eSetValueWithOverwrite);

    cli->serial->print("LED pattern set to ");
    cli->serial->println(pattern);
}

static void CLI_IRcode(CLI_Object *cli, uint8_t nargs, char **args)
{
    cli->serial->print("Last Code Received: ");
    cli->serial->printf("%X\n",cli->ir->last_code);
}