#pragma once

// i2c pins
#define I2C_SDA_PIN 23u
#define I2C_SCL_PIN 18u

// led driver pins
#define LED_SDB_PIN 22u

// touch buttons
#define DOWN_TOUCH T5
#define LEFT_TOUCH T2
#define UP_TOUCH T3
#define RIGHT_TOUCH T4
#define SELECT_TOUCH T6

// buzzer
#define BUZZER_PIN 5
#define BUZZER_CHANNEL 2
#define BUZZER_DEFAULT_FREQ 4000
#define BUZZER_RESOLUTION 10

// EEPROM addresses
#define EEPROM_SIZE 0x40
#define EEPROM_ADDR_LED_MODE 0x20 // 8 bit
#define EEPROM_ADDR_LED_BRIGHT 0x21 // 8 bit
#define EEPROM_ADDR_LED_DELAY 0x22 // 16 bit
#define EEPROM_ADDR_DEV_MODE 0x24 // 8 bit
#define EEPROM_ADDR_TAG_LIVES 0x25 // one byte
#define EEPROM_ADDR_TAG_DAMAGE 0x26 // one byte, for now

// IR
#define IR_TX_PIN 4u
#define IR_RX_PIN 25u
#define PULSE_BUFFER_SIZE 66u
#define TOLERANCE 0.25 // how much play we want within pulse measurements

// networking
#define IRC_SERVER "irl.depaul.edu"
#define IRC_PORT 6667
#define IRC_USER "TC0xCBADGE"
#define NTP_SERVER "pool.ntp.org"