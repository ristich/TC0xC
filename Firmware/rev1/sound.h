#include <Arduino.h>

// buzzers
#define BUZZER_PIN      5
#define BUZZER_CHANNEL   0


// each soundbyte in a call
void base(uint8_t buzzerPin); // Contra's base tunes
void opening(uint8_t buzzerPin); // Contra opening theme
void roll(uint8_t buzzerPin); // rick roll
void game_over(uint8_t buzzerPin); // Contra game over
void boss(uint8_t buzzerPin); // Contra boss theme
void pure_energy(uint8_t buzzerPin); //Pure Energy