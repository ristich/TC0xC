#include "IRCClient.h"
#include <sys/_stdint.h>
#pragma once

#include <Arduino.h>

#define MAX_HEALTH 254
#define LIVES 9


typedef struct {
    char id[9];
    uint8_t health;
    uint8_t livesRemaining;
    uint16_t ammo;
    TaskHandle_t recordHit;
    IRCClient *irc; 
} Player_Object;

void Player_init(Player_Object* player, const char* playerName, uint8_t initialHealth, uint8_t initialLives);
