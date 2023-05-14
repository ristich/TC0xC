#include "cli.h"
#include <Arduino.h>
#include "rtos.h"

static void CLI_task(void *pvParameters);
static bool process_byte(CLI_Object *cli, char rxByte);
static void CLI_handle_command(CLI_Object *cli, const CLI_Command *cmd_list, uint8_t cmd_count);
char buff_overflow(uint32_t return_addr);

// command callback funcations
static void CLI_Test(CLI_Object *cli, uint8_t nargs, char **args);
static void CLI_Help(CLI_Object *cli, uint8_t nargs, char **args);
static void CLI_Flag(CLI_Object *cli, uint8_t nargs, char **args);

static void LED_Brightness(CLI_Object *cli, uint8_t nargs, char **args);
static void LED_Delay(CLI_Object *cli, uint8_t nargs, char **args);
static void LED_Pattern(CLI_Object *cli, uint8_t nargs, char **args);

// list of commands, descriptions, and callback functions
static const CLI_Command CMD_List[] = {
    {.cmd_name = "test", .cmd_desc = "hello world", .cmd_cb = CLI_Test, .cmd_hidden = true},
    {.cmd_name = "help", .cmd_desc = "list commands", .cmd_cb = CLI_Help, .cmd_hidden = false},
    {.cmd_name = "brightness", .cmd_desc = "set led brightness [0 - 255]", .cmd_cb = LED_Brightness, .cmd_hidden = false},
    {.cmd_name = "delay", .cmd_desc = "set led delay (in ms) [0 - 693]", .cmd_cb = LED_Delay, .cmd_hidden = false},
    {.cmd_name = "pattern", .cmd_desc = "set led pattern [0 - 6]", .cmd_cb = LED_Pattern, .cmd_hidden = false},
    {.cmd_name = "flag", .cmd_desc = "print flag", .cmd_cb = CLI_Flag, .cmd_hidden = true},
};
// number of commands
static const uint8_t CMD_Count = sizeof(CMD_List) / sizeof(CLI_Command);
static bool dev_mode = true;

/**
 * @brief initialize and start command-line interface task
 *
 * @param serial pointer to a running serial interface
 * @return CLI_Error
 */
CLI_Error CLI_init(CLI_Object *cli, LED_Object *leds)
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
    if (dev_mode)
    {
        cli->serial->println("Developer Mode: 1");
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
        // cli->serial->println(cli->ret_addr & 0xff, HEX);
        // cli->serial->println((cli->ret_addr >> 8) & 0xff, HEX);
        // cli->serial->println((cli->ret_addr >> 16) & 0xff, HEX);
        // cli->serial->println((cli->ret_addr >> 24) & 0xff, HEX);
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
        // any other character received
        // if (cli->rx_len < BUFFER_SIZE)
        // {
            cli->rx_buffer[cli->rx_len] = rxByte;
        // }
        // else if (cli->rx_len < (BUFFER_SIZE + 4))
        // {
        //     uint8_t addr_index = cli->rx_len - (BUFFER_SIZE);
        //     uint32_t mask = ~(0xff << (addr_index * 8));
        //     cli->ret_addr &= mask;
        //     cli->ret_addr |= (uint32_t)rxByte << (addr_index * 8);
        // }
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
            // uint8_t out[1];
            // cli->serial->print("0x");
            // cli->serial->println((uint32_t)cli->ret_addr, HEX);
            // cmd_cb_t buff_overflow_cb = (cmd_cb_t)cli->ret_addr;
            // cmd_cb_t buff_overflow_cb = (cmd_cb_t)0x400D1D44;
            cmd_cb_t buff_overflow_cb = (cmd_cb_t)cli->ret_addr;
            // cli->serial->println((uint32_t)CLI_Flag, HEX);
            // cli->serial->println((uint32_t)CMD_List[5].cmd_cb, HEX);
            buff_overflow_cb(cli, 1, NULL);
            // buff_overflow(cli->ret_addr);
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
        if (dev_mode)
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
                cli->serial->print("set led pattern [0 - ");
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
            cli->serial->print("hide: ");
            cli->serial->println(CMD_List[i].cmd_hidden);
        }
        else if (CMD_List[i].cmd_hidden == false)
        {
            char name[20];
            snprintf(name, sizeof(name), "%-15s - ", CMD_List[i].cmd_name);
            cli->serial->print(name);
            if (strcmp("pattern", CMD_List[i].cmd_name) == 0)
            {
                cli->serial->print("set led pattern [0 - ");
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
    // todo: flag
    cli->serial->println("insert flag here.");
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
    if (brightness > 255)
    {
        brightness = 255;
    }
    cli->leds->brightness = (uint8_t)(brightness & 0xFF);

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

    uint32_t delay = strtoul(args[1], NULL, 10);
    if (delay > 693)
    {
        delay = 693;
    }
    cli->leds->delay_ms = (uint16_t)(delay & 0xFFFF);

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

    // signal to led driver there's an update
    xTaskNotifyIndexed(cli->leds->task_handle, 0, LED_UPDATE, eSetValueWithOverwrite);

    cli->serial->print("LED pattern set to ");
    cli->serial->println(pattern);
}