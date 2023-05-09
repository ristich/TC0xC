#pragma once

// i2c pins
#define I2C_SDA 23u
#define I2C_SCL 18u

// led driver pins
#define LED_SDB 22u

// touch buttons
#define DOWN_TOUCH T5
#define LEFT_TOUCH T2
#define UP_TOUCH T3
#define RIGHT_TOUCH T4
#define SELECT_TOUCH T6

// buzzer
#define BUZZER_PIN 5
#define BUZZER_CHANNEL 0
#define BUZZER_DEFAULT_FREQ 4000
#define BUZZER_RESOLUTION 10

// irc specs 
#define IRC_SERVER   "irl.depaul.edu"  // Edit These!
#define IRC_PORT     6667
#define IRC_USER "TC0XCBADGE"
#define REPLY_TO "rudy"