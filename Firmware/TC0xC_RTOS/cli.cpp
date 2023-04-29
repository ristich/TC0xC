#include "cli.h"

static void CLI_task(void *pvParameters);
static bool process_byte(CLI_Object *cli_obj, char rxByte);
static void CLI_handle_command(CLI_Object *cli_obj, const CLI_Command *cmd_list, uint8_t cmd_count);

// command callback funcations
static void CLI_Test(CLI_Object *cli_obj, uint8_t nargs, char **args);
static void CLI_Help(CLI_Object *cli_obj, uint8_t nargs, char **args);

static void LED_Brightness(CLI_Object *cli_obj, uint8_t nargs, char **args);
static void LED_Delay(CLI_Object *cli_obj, uint8_t nargs, char **args);

// list of commands, descriptions, and callback functions
static const CLI_Command CMD_List[] = {
    {.cmd_name = "test", .cmd_desc = "hello world", .cmd_cb = CLI_Test},
    {.cmd_name = "help", .cmd_desc = "list commands", .cmd_cb = CLI_Help},
    {.cmd_name = "brightness", .cmd_desc = "set led brightness", .cmd_cb = LED_Brightness},
    {.cmd_name = "delay", .cmd_desc = "set led delay (in ms)", .cmd_cb = LED_Delay},
};
// number of commands
static const uint8_t CMD_Count = sizeof(CMD_List) / sizeof(CLI_Command);

/**
 * @brief initialize and start command-line interface task
 *
 * @param serial pointer to a running serial interface
 * @return CLI_Error
 */
CLI_Error CLI_init(HardwareSerial *serial, LED_Object *leds)
{
    static bool initialized = false;
    if (initialized)
    {
        return CLI_SUCCESS;
    }

    // create CLI object to pass around for serial and buffer acccess
    static CLI_Object cli_obj{
        .leds = leds,
        .serial = serial,
        .rx_buffer = {0},
        .rx_len = 0,
    };

    xTaskCreatePinnedToCore(CLI_task, "CLI_task", 2048, &cli_obj, tskIDLE_PRIORITY + 3, NULL, app_cpu);

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
    CLI_Object *cli_obj = (CLI_Object *)pvParameters;
    cli_obj->serial->println("\nWelcome to CLI");
    cli_obj->serial->print("CMD count: ");
    cli_obj->serial->println(CMD_Count);

    while (1)
    {
        if (cli_obj->serial->available())
        {
            char r = cli_obj->serial->read();
            if (process_byte(cli_obj, r))
            {
                // command received
                CLI_handle_command(cli_obj, CMD_List, CMD_Count);

                // clear command buffer
                cli_obj->rx_len = 0;
                cli_obj->rx_buffer[0] = 0;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief process byte received by serial interface
 *
 * @param cli_obj pointer to cli object
 * @param rxByte serial byte received
 * @return true complete command was received
 * @return false buffer is still accumulating
 */
static bool process_byte(CLI_Object *cli_obj, char rxByte)
{
    if ((rxByte == '\r') || (rxByte == '\n'))
    {
        // command entered
        cli_obj->rx_buffer[cli_obj->rx_len] = 0;
        cli_obj->serial->print(rxByte);
        cli_obj->rx_len = 0;
        return true;
    }
    if ((rxByte == 0x7f) || (rxByte == 0x08))
    {
        // backspace or delete
        if (cli_obj->rx_len > 0)
        {
            cli_obj->rx_len--;
            cli_obj->rx_buffer[cli_obj->rx_len] = 0;
        }
        cli_obj->serial->print(rxByte);
    }
    else
    {
        // any other character received
        cli_obj->rx_buffer[cli_obj->rx_len] = rxByte;
        cli_obj->rx_len++;
        if (cli_obj->rx_len >= BUFFER_SIZE)
        {
            // probably put a flag here for pretend buffer overflow
            cli_obj->rx_len = BUFFER_SIZE - 1;
            char backspace = 0x08;
            cli_obj->serial->print(backspace);
        }
        cli_obj->serial->print(rxByte);
    }
    return false;
}

/**
 * @brief validate and handle command
 *
 * @param cli_obj pointer to CLI object
 * @param cmd_list array of CLI commands
 * @param cmd_count number of CLI commands
 */
static void CLI_handle_command(CLI_Object *cli_obj, const CLI_Command *cmd_list, uint8_t cmd_count)
{
    bool command_found = false;
    uint8_t nargs = 0;
    char *pch;
    char *args[CLI_ARG_COUNT_MAX];

    // parse arguments
    args[nargs] = cli_obj->rx_buffer;
    pch = strpbrk(cli_obj->rx_buffer, " ");
    nargs++;
    if (pch)
    {
        *pch = '\0';
    }

    for (int i = 0; i < cmd_count; i++)
    {
        if (strcmp((char *)cli_obj->rx_buffer, cmd_list[i].cmd_name) == 0)
        {
            command_found = true;
            // cli_obj->serial->print("command found: ");
            // cli_obj->serial->println(cli_obj->rx_buffer);
            while (pch != NULL)
            {
                args[nargs] = pch + 1;
                *pch = '\0';
                pch = strpbrk(cli_obj->rx_buffer, " ");
                nargs++;
                if (nargs >= CLI_ARG_COUNT_MAX)
                {
                    break;
                }
            }

            if (cmd_list[i].cmd_cb != NULL)
            {
                cmd_list[i].cmd_cb(cli_obj, nargs - 1, args);
            }
        }
    }

    if (command_found == false)
    {
        cli_obj->serial->print("ERR: unknown command '");
        cli_obj->serial->print(cli_obj->rx_buffer);
        cli_obj->serial->println("'");
    }
}

/**
 * @brief test CLI command callback
 *
 * @param cli_obj pointer to CLI object
 * @param nargs number of arguments
 * @param args array of arguments
 */
static void CLI_Test(CLI_Object *cli_obj, uint8_t nargs, char **args)
{
    cli_obj->serial->println("CLI test function");
}

/**
 * @brief help CLI command callback
 *
 * @param cli_obj pointer to CLI object
 * @param nargs number of arguments
 * @param args array of arguments
 */
static void CLI_Help(CLI_Object *cli_obj, uint8_t nargs, char **args)
{
    cli_obj->serial->println("CLI help function");
    for (int i = 0; i < CMD_Count; i++)
    {
        char name[20];
        snprintf(name, sizeof(name), "%-15s - ", CMD_List[i].cmd_name);
        cli_obj->serial->print(name);
        cli_obj->serial->println(CMD_List[i].cmd_desc);
    }
}

static void LED_Brightness(CLI_Object *cli_obj, uint8_t nargs, char **args)
{
    if (nargs != 1)
    {
        cli_obj->serial->print("ERR: expected 1 argument, received ");
        cli_obj->serial->println(nargs);
        return;
    }

    uint8_t brightness = strtol(args[1], NULL, 10);
    cli_obj->leds->brightness = brightness;

    cli_obj->serial->print("LED brightness set to: ");
    cli_obj->serial->println(brightness);
}

static void LED_Delay(CLI_Object *cli_obj, uint8_t nargs, char **args)
{
    if (nargs != 1)
    {
        cli_obj->serial->print("ERR: expected 1 argument, received ");
        cli_obj->serial->println(nargs);
        return;
    }

    uint32_t delay = strtoul(args[1], NULL, 10);
    cli_obj->leds->delay_ms = delay;

    cli_obj->serial->print("LED delay set to: ");
    cli_obj->serial->print(delay);
    cli_obj->serial->println("ms");
}