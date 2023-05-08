#include <WiFi.h>
#include <IRCClient.h>
#include "rtos.h"
#include "led.h"
#include "cli.h"
#include "sound.h"

const char* ssid = "FACEPLANT";
const char* pass = "FACEPLANT";

char NICK[9] = "t0XXXXXX";
uint32_t nextCheckTime = 0;

WiFiClient wClient;
IRCClient client(IRC_SERVER, IRC_PORT, wClient);

typedef struct
{
    LED_Object leds;
    CLI_Object cli;
} Badge_Object;

Badge_Object Badge = {0};


void command_handler(IRCMessage ircMessage) {
  // PRIVMSG ignoring CTCP messages
  if (ircMessage.command == "PRIVMSG" && ircMessage.text[0] != '\001') {
    String message("<" + ircMessage.nick + "> " + ircMessage.text);
    Serial.println(message);
  }
  
  // reset command
  if (ircMessage.text == "!restart") {
     client.sendMessage(ircMessage.parameters,  String(NICK) + " restarting in 3 seconds");
     client.sendRaw("PART #dev");
     client.sendRaw("QUIT");
     delay(3000);
     ESP.restart();
  }

  // uptime (in clock cycles)
   if (ircMessage.text == "!uptime") {
     client.sendMessage(ircMessage.parameters, "live for "+ String(ESP.getCycleCount()) + " cycles");
  }

  // report sketch versions and size
  if (ircMessage.text == "!firmware") {
    client.sendMessage(ircMessage.parameters, "hash " + ESP.getSketchMD5() + " " + ESP.getSketchSize() +" bytes");
  }  

  if (ircMessage.text == "!boot") 
  {
       boss(BUZZER_PIN); 
       client.sendMessage(ircMessage.parameters, "playing boot animation");   
  }

  // if (ircMessage.text == "!roll") 
  // {
  //      rollEm(tcleds, 0);
  //      client.sendMessage(ircMessage.parameters, "keep on rollin' babeh");   
  // }

  // if (ircMessage.text == "!alert") 
  // {
  //       alert(tcleds);
  //       client.sendMessage(ircMessage.parameters, "alert! missing child");   
  //       tcleds.setTextBotColor(0x01, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextTopColor(0x01, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextBotColor(0x64, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextTopColor(0x64, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextBotColor(0x20, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextTopColor(0x20, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextBotColor(0x10, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextTopColor(0x10, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextBotColor(0x08, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextTopColor(0x08, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextBotColor(0x04, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextTopColor(0x04, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextBotColor(0x02, random(0,255), random(0,255), random(0,255));
  //       tcleds.setTextTopColor(0x02, random(0,255), random(0,255), random(0,255));
  // }

  if(ircMessage.text == "!update")
     {
      client.sendMessage(ircMessage.parameters, "attempting update");
      //execOTA();
      delay(10000);
      client.sendMessage(ircMessage.parameters, "update failed");
     }
}

void debugSentCallback(String data) {
  Serial.println(data);
}

void setup()
{
    // task initializations
    pinMode(BUZZER_PIN,OUTPUT);
    LED_init(&Badge.leds);
    CLI_init(&Badge.cli, &Badge.leds);
    //open(BUZZER_PIN);
      // Wifi
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
  sprintf(NICK, "t%06X", (uint32_t)(ESP.getEfuseMac() >> 24));
  client.setCallback(command_handler);
  client.setSentCallback(debugSentCallback); 


}

void loop()
{
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if (!client.connected()) {
    if (millis() > nextCheckTime)
    {
      Serial.println("Attempting IRC connection...");
      // Attempt to connect
      if (client.connect(NICK, IRC_USER)) {
        Serial.println("connected");
        delay(3000);
        client.sendRaw("JOIN #dev RR");
      } else {
        Serial.println("failed... try again in 2 minutes");
        // Wait 5 seconds before retrying
        nextCheckTime = millis() + 120000; // try in 2 min
      }
    }
  }

  client.loop();
}
