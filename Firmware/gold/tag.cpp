
#include <Arduino.h>
#include "tag.h"


void initializePlayer(Player_Object* player, const char* playerName, uint8_t initialHealth, uint8_t initialLives){
    strncpy(player->id, playerName, 9);
    player->health = initialHealth;
    player->damageList = NULL;
    player->livesRemaining = initialLives;
}

DamageNode* createDamageNode(uint8_t amount, const char* attacker) {
    DamageNode* node = (DamageNode*)malloc(sizeof(DamageNode));
    node->amount = amount;
    strcpy(node->attacker, attacker);
    node->next = NULL;
    return node;
}

void addDamage(Player_Object* player, uint8_t amount, const char* attacker) {
    DamageNode* newNode = createDamageNode(amount, attacker);

    if (player->damageList == NULL) {
        player->damageList = newNode;
    } else {
        DamageNode* currentNode = player->damageList;
        while (currentNode->next != NULL) {
            currentNode = currentNode->next;
        }
        currentNode->next = newNode;
    }
}

void takeDamage(Player_Object* player, const char* attacker, uint8_t amount) {
    player->health -= amount;
    addDamage(player, amount, attacker);

    if (player->health <= 0) {
        player->health = 0;
        player->livesRemaining--;

        if (player->livesRemaining > 0) {
            printf("%s has been eliminated by %s. %d lives remaining.\n", player->id, player->damageList->attacker,
                   player->livesRemaining);
            resetPlayer(player);
        } else {
            printf("%s has been eliminated. Game over!\n", player->id);
        }
    } else {
        printf("%s has been hit by %s for %d damage. Remaining health: %d\n", player->id, attacker, amount,
               player->health);
    }
}

void resetPlayer(Player_Object* player) {
    player->health = 100;

    // Free the damage linked list
    DamageNode* currentNode = player->damageList;
    while (currentNode != NULL) {
        DamageNode* nextNode = currentNode->next;
        free(currentNode);
        currentNode = nextNode;
    }
    player->damageList = NULL;
}

void reportDamageList(const Player_Object* player) {
    printf("Damage inflicted on %s:\n", player->id);

    DamageNode* currentNode = player->damageList;
    while (currentNode != NULL) {
        printf("  Attacker: %s, Amount: %d\n", currentNode->attacker, currentNode->amount);
        currentNode = currentNode->next;
    }
}