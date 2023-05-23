#include <WiFi.h>
#include <EEPROM.h>
#include <IRCClient.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Update.h>
#include <time.h>
#include "rtos.h"
#include "led.h"
#include "cli.h"
#include "sound.h"
#include "touch.h"
#include "audio.h"
#include "tag.h"
#include "ir.h"

const char* ssid = "THOTCON-Open";
const char* pass = NULL;


char NICK[9] = "t0XXXXXX";
uint32_t nextCheckTime = 0;

struct tm timeinfo;
WiFiUDP Udp; 
WiFiClient wClient;
IRCClient client(IRC_SERVER, IRC_PORT, wClient);

typedef enum {
    MAIN_MENU = 0,
    TAG,
    INFRARED_COPY,
    MULTI_MEDIA,
    INFRARED_PLAY
} State;

typedef struct
{
    LED_Object leds;
    CLI_Object cli;
    Audio_Object audio;
    Touch_Object touch;
    Player_Object player;
    IR_Object ir;
    State state;
} Badge_Object;

Badge_Object Badge = {0};

IRsend irsend(IR_TX_PIN);

void setup()
{
    //variable builds
    sprintf(NICK, "TC%06X", (uint32_t)(ESP.getEfuseMac() >> 24));
        // check for dev mode
    EEPROM.begin(EEPROM_SIZE);
    delay(500);
    uint8_t dev_read = EEPROM.readByte(EEPROM_ADDR_DEV_MODE);
    if (dev_read > 1)
    {
        EEPROM.writeByte(EEPROM_ADDR_DEV_MODE, 0);
        EEPROM.commit();
    }
    else if (dev_read == 1)
    {
        Badge.cli.dev_mode = 1;
    }
    delay(3000);
    // Badge object initializations
    Serial.begin(115200);
   
    vTaskDelay(500);
    LED_init(&Badge.leds);
    Player_init(&Badge.player, NICK, MAX_HEALTH, LIVES);
    vTaskDelay(500);
    audio_init(&Badge.audio);
    vTaskDelay(500);
    CLI_init(&Badge.cli, &Badge.leds, &Badge.ir);
    
    //touch_init here fails an assert as soon as wifi connects
    Badge.state = MAIN_MENU;
    // Opening animations here
      // Wifi

   //rtc.setTime(readLinuxEpochUsingNTP());

  Serial.println();
  Serial.println();
  vTaskDelay(3000);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
  
 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.macAddress());  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 
  Badge.player.irc->setCallback(command_handler);
  //Badge.player.irc->setSentCallback(debugSentCallback); 
  Serial.print("BadgeBattles Soldier ");
  Serial.print(Badge.player.id);
  Serial.print(" locked and loaded @ ");
  configTime((-5*60*60), 0, NTP_SERVER);
  printLocalTime();
 
 
  // initializing ir & touch here makes the system have fewer timing conflicts.
  ir_init(&Badge.ir,Badge.player.irc);
  touch_init(&Badge.touch, &Badge.cli, Badge.leds.task_handle, Badge.audio.task_handle, Badge.ir.tx_task_handle);
 
}

void loop()
{
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if (!Badge.player.irc->connected()) {
    if (millis() > nextCheckTime)
    {
      Serial.print("Attempting BadgeNET connection... ");
      // Attempt to connect
      if (Badge.player.irc->connect(NICK, IRC_USER)) {
        Serial.println("connected");
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        Badge.player.irc->sendRaw("JOIN #jungle"); 
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        Badge.player.irc->sendRaw("JOIN #bb23");
      } else {
        Serial.println("failed... try again in 2 minutes");
        // Wait 5 seconds before retrying
        nextCheckTime = millis() + 120000; // try in 2 min
      }
    }
  }

  Badge.player.irc->loop();                                                                      
  
}
