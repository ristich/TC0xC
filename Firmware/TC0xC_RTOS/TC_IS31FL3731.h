#pragma once

#include <Arduino.h>
#include <Wire.h>

#define IC_3728 0u
#define IC_3731 1u

#define ISSI_ADDR_DEFAULT 0x74

#define ISSI_REG_CONFIG 0x00
#define ISSI_REG_CONFIG_PICTUREMODE 0x00
#define ISSI_REG_CONFIG_AUTOPLAYMODE 0x08
#define ISSI_REG_CONFIG_AUDIOPLAYMODE 0x18

#define ISSI_CONF_PICTUREMODE 0x00
#define ISSI_CONF_AUTOFRAMEMODE 0x04
#define ISSI_CONF_AUDIOMODE 0x08

#define ISSI_REG_PICTUREFRAME 0x01

#define ISSI_REG_SHUTDOWN 0x0A
#define ISSI_REG_AUDIOSYNC 0x06

#define ISSI_COMMANDREGISTER 0xFD
#define ISSI_BANK_FUNCTIONREG 0x0B // aka 'page nine' :face-palm:

// LED position defines
#define LED_UP1 1u
#define LED_UP2 2u
#define LED_UP3 3u
#define LED_UP4 4u
#define UP_LED 1u
#define RIGHT_LED 2u
#define DOWN_LED 3u
#define LEFT_LED 4u
#define SELECT_LED 5u
#define START_LED 6u
#define B_LED 7u
#define A_LED 8u
#define SEAT1_LED 9u
#define SEAT2_LED 10u
#define SEAT3_LED 11u
#define SEAT4_LED 12u
#define SEAT5_LED 13u

typedef enum
{
  LED_COL_TWELVE = 0,
  LED_COL_ONE,
  LED_COL_FIVE,
  LED_COL_SIX,
  LED_COL_SEVEN,
  LED_COL_ELEVEN,
} led_col_t;

/**************************************************************************/
/*!
    @brief Constructor for generic IS31FL3731 breakout version
*/
/**************************************************************************/
class TC_IS31FL3731
{
public:
  TC_IS31FL3731(uint8_t driver = IC_3731) { _ledDriver = driver; }
  bool begin(uint8_t sdaPin, uint8_t sclPin, uint8_t addr = ISSI_ADDR_DEFAULT);
  void clear(void);

  void setLEDPWM(uint8_t lednum, uint8_t pwm, uint8_t bank = 0);
  void setAllLEDPWM(uint8_t pwm, uint8_t bank = 0);
  void setColumn(led_col_t col, uint8_t pwm, uint8_t bank = 0);
  void setLED(uint8_t led, uint8_t set, uint8_t bank = 0);

  void audioSync(bool sync);
  void setFrame(uint8_t b);
  void displayFrame(uint8_t frame);
  void writeRegister8(uint8_t bank, uint8_t reg, uint8_t data);

protected:
  void selectBank(uint8_t bank);
  // void writeRegister8(uint8_t bank, uint8_t reg, uint8_t data);
  uint8_t readRegister8(uint8_t bank, uint8_t reg);
  uint8_t _ledDriver;
  uint8_t _i2caddr; ///< The I2C address we expect to find the chip
  uint8_t _frame;   ///< The frame (of 8) we are currently addressing
};
