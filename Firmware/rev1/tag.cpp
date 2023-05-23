#include "esp_compiler.h"
#include "esp32-hal-uart.h"
#include "hal.h"

#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <IRCClient.h>
#include "tag.h"

WiFiClient wfClient;
IRCClient irc_client(IRC_SERVER, IRC_PORT, wfClient);


void Player_init(Player_Object* player, const char* playerName, uint8_t initialHealth, uint8_t initialLives){
    strncpy(player->id, playerName, 9);
    if ( EEPROM.readByte(EEPROM_ADDR_TAG_LIVES) == 0xFF ){ //probably stock 
        player->livesRemaining = initialLives;
        EEPROM.writeByte(EEPROM_ADDR_TAG_LIVES, initialLives);
    } else {
      player->livesRemaining = EEPROM.readByte(EEPROM_ADDR_TAG_LIVES);
    } 
    
    if ( EEPROM.readByte(EEPROM_ADDR_TAG_DAMAGE) == 0xFF ){ //probably stock 
        player->health = initialHealth;
        EEPROM.writeByte(EEPROM_ADDR_TAG_DAMAGE, initialHealth);
    } else {
      player->health = EEPROM.readByte(EEPROM_ADDR_TAG_DAMAGE);
    } 

    player->ammo = 1000; // reboot to reload

    player->irc = &irc_client;
    
    Serial.printf("Soldier %s enters the game. [%d lives : %d health : %d ammo]\n",player->id,player->livesRemaining,player->health,player->ammo);
    
}

void reportDamageTask(const Player_Object* player, uint32_t ) {
    printf("Damage inflicted on %s:\n", player->id);

    // DamageNode* currentNode = player->damageList;
    // while (currentNode != NULL) {
    //     printf("  Attacker: %s, Amount: %d\n", currentNode->attacker, currentNode->amount);
    //     currentNode = currentNode->next;
    // }
}