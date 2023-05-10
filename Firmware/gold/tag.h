#include <sys/_stdint.h>
#pragma once

#include <Arduino.h>

#define MAX_HEALTH 255
#define LIVES 9

typedef struct DamageNode {
    uint8_t amount;
    char attacker[9];
    struct DamageNode* next;
} DamageNode;

typedef struct {
    char id[9];
    uint8_t health;
    DamageNode* damageList;
    uint8_t livesRemaining;
} Player_Object;

void Player_init(Player_Object* player, const char* playerName, uint8_t initialHealth, uint8_t initialLives);
DamageNode* createDamageNode(uint8_t amount, const char* attacker);
void addDamage(Player_Object* player, uint8_t amount, const char* attacker);
void takeDamage(Player_Object* player, const char* attacker, uint8_t amount);
void resetPlayer(Player_Object* player);
void reportDamageList(const Player_Object* player);