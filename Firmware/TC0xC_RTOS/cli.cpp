#include "cli.h"

static void CLI_task(void *pvParameters);
static bool process_byte(CLI_Object *cli, char rxByte);
static void CLI_handle_command(CLI_Object *cli, const CLI_Command *cmd_list, uint8_t cmd_count);

// command callback funcations
static void CLI_Test(CLI_Object *cli, uint8_t nargs, char **args);
static void CLI_Help(CLI_Object *cli, uint8_t nargs, char **args);

static void LED_Brightness(CLI_Object *cli, uint8_t nargs, char **args);
static void LED_Delay(CLI_Object *cli, uint8_t nargs, char **args);

// list of commands, descriptions, and callback functions
static const CLI_Command CMD_List[] = {
    {.cmd_name = "test", .cmd_desc = "hello world", .cmd_cb = CLI_Test},
    {.cmd_name = "help", .cmd_desc = "list commands", .cmd_cb = CLI_Help},
    {.cmd_name = "brightness", .cmd_desc = "set led brightness [0 - 255]", .cmd_cb = LED_Brightness},
    {.cmd_name = "delay", .cmd_desc = "set led delay (in ms) [0 - 693]", .cmd_cb = LED_Delay},
};
// number of commands
static const uint8_t CMD_Count = sizeof(CMD_List) / sizeof(CLI_Command);

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

    xSemaphoreTake(leds->update_sem, portMAX_DELAY);

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
    cli->serial->print("CMD count: ");
    cli->serial->println(CMD_Count);

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
    if ((rxByte == '\r') || (rxByte == '\n'))
    {
        // command entered
        cli->rx_buffer[cli->rx_len] = 0;
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
            cli->rx_buffer[cli->rx_len] = 0;
        }
        cli->serial->print(rxByte);
    }
    else
    {
        // any other character received
        cli->rx_buffer[cli->rx_len] = rxByte;
        cli->rx_len++;
        if (cli->rx_len >= BUFFER_SIZE)
        {
            // probably put a flag here for pretend buffer overflow
            cli->rx_len = BUFFER_SIZE - 1;
            char backspace = 0x08;
            cli->serial->print(backspace);
        }
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
            // cli->serial->print("command found: ");
            // cli->serial->println(cli->rx_buffer);
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
        cli->serial->print("ERR: unknown command '");
        cli->serial->print(cli->rx_buffer);
        cli->serial->println("'");
    }
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
    cli->serial->println("CLI help function");
    for (int i = 0; i < CMD_Count; i++)
    {
        char name[20];
        snprintf(name, sizeof(name), "%-15s - ", CMD_List[i].cmd_name);
        cli->serial->print(name);
        cli->serial->println(CMD_List[i].cmd_desc);
    }
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
    xSemaphoreGive(cli->leds->update_sem);
    vTaskDelay(10);
    xSemaphoreTake(cli->leds->update_sem, portMAX_DELAY);

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
    xSemaphoreGive(cli->leds->update_sem);
    vTaskDelay(10);
    xSemaphoreTake(cli->leds->update_sem, portMAX_DELAY);

    cli->serial->print("LED delay set to ");
    cli->serial->print(delay);
    cli->serial->println("ms");
}