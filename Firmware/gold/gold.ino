#include <WiFi.h>
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

const char* ssid = "FACEPLANT"; //TODO Update for Con Wifi
const char* pass = "FACEPLANT";

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
    delay(3000);
    // Badge object initializations
    pinMode(BUZZER_PIN,OUTPUT);
    LED_init(&Badge.leds);
    CLI_init(&Badge.cli, &Badge.leds);
    Player_init(&Badge.player, NICK, MAX_HEALTH, LIVES);
    audio_init(&Badge.audio);
    
    //touch_init here fails an assert as soon as wifi connects
    Badge.state = MAIN_MENU;
    // Opening animations here
      // Wifi

   //rtc.setTime(readLinuxEpochUsingNTP());

  Serial.println();
  Serial.println();
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
  client.setCallback(command_handler);
  client.setSentCallback(debugSentCallback); // todo comment out for final bin
  Serial.print("Contra Player ");
  Serial.print(Badge.player.id);
  Serial.print(" locked and loaded @ ");
  configTime((-5*60*60), 0, NTP_SERVER);
  printLocalTime();
 
 // xTaskNotifyIndexed(Badge.audio.task_handle, 0, HIT_SONG, eSetValueWithOverwrite); 
  // initializing touch here makes the system "stable" until a touch is handled then fails: assert failed: xTaskGenericNotify tasks.c:5545 (xTaskToNotify)
  ir_init(&Badge.ir);
  touch_init(&Badge.touch, &Badge.cli, Badge.leds.task_handle, Badge.audio.task_handle, Badge.ir.tx_task_handle);

}

void loop()
{
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if (!client.connected()) {
    if (millis() > nextCheckTime)
    {
      Serial.print("Attempting BadgeNET connection... ");
      // Attempt to connect
      if (client.connect(NICK, IRC_USER)) {
        Serial.println("connected");
        delay(3000);
        client.sendRaw("JOIN #dev RR"); //todo pick a initial channel name
      } else {
        Serial.println("failed... try again in 2 minutes");
        // Wait 5 seconds before retrying
        nextCheckTime = millis() + 120000; // try in 2 min
      }
    }
  }

  client.loop();
}
