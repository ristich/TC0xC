#pragma once

#include <Arduino.h>
#include <Wire.h>

#define IC_3728 0u
#define IC_3731 1u

#define ISSI_ADDR_DEFAULT 0x74

#define ISSI_REG_CONFIG 0x00
#define CONFIG_MODE_SHIFT 3u
#define CONFIG_MODE_MASK 0x18
#define CONFIG_MODE_PICTURE (0x00 << CONFIG_MODE_SHIFT)
#define CONFIG_MODE_AUTOPLAY (0x01 << CONFIG_MODE_SHIFT)
#define CONFIG_MODE_AUDIOPLAY (0x10 << CONFIG_MODE_SHIFT)
#define CONFIG_FRAME_SHIFT 0u
#define CONFIG_FRAME_MASK 0x07

#define ISSI_REG_PICTURE_DISPLAY 0x01
#define PICTURE_FRAME_MASK 0x07

#define ISSI_REG_AUTO_CONTROL1 0x02
#define AUTO_CONTROL_LOOPS_SHIFT 4u
#define AUTO_CONTROL_LOOPS_MASK 0x70
#define AUTO_CONTROL_FRAMES_SHIFT 0u
#define AUTO_CONTROL_FRAMES_MASK 0x07

#define ISSI_REG_AUTO_CONTROL2 0x03
#define AUTO_CONTROL_DELAY_SHIFT 0u
#define AUTO_CONTROL_DELAY_MASK 0x3F

#define ISSI_REG_SHUTDOWN 0x0A
#define ISSI_REG_AUDIOSYNC 0x06

#define ISSI_COMMANDREGISTER 0xFD
#define ISSI_BANK_FUNCTIONREG 0x0B // aka 'page nine' :face-palm:

typedef enum
{
    LED_COL_TWELVE = 0,
    LED_COL_ONE,
    LED_COL_FIVE,
    LED_COL_SIX,
    LED_COL_SEVEN,
    LED_COL_ELEVEN,
} led_col_t;

// 0 - 23 as they appear on plastics
const uint8_t led_addrs[24] = {0x57, 0x47, 0x37, 0x27, 0x58, 0x48, 0x28, 0x38,
                               0x59, 0x49, 0x39, 0x29, 0x54, 0x44, 0x34, 0x24,
                               0x55, 0x45, 0x35, 0x25, 0x56, 0x46, 0x36, 0x26};

// Function settings
typedef enum
{
    Display_Mode_Picture,
    Display_Mode_Auto_Play,
    Display_Mode_Audio_Play,
} Display_Mode;

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

    void setDisplayMode(Display_Mode mode);

    void setPictureFrame(uint8_t frame);

    void setAutoPlayStart(uint8_t frame);
    void setAutoPlayFrames(uint8_t frames);
    void setAutoPlayLoops(uint8_t loops);
    void setAutoPlayDelay(uint16_t delay_ms);

    void audioSync(bool sync);

    void clear(uint8_t bank = 0);
    void setLEDPWM(uint8_t led, uint8_t pwm, uint8_t bank = 0);
    void setAllLEDPWM(uint8_t pwm, uint8_t bank = 0);
    void setColumn(led_col_t col, uint8_t pwm, uint8_t bank = 0);
    void setBadgeLED(uint8_t lednum, uint8_t set, uint8_t bank = 0);
    void setBadgeLEDs(uint32_t config, uint8_t pwm, uint8_t bank = 0);
    void setBadgeLetter(char letter, uint8_t pwm, uint8_t bank = 0);
    void setBadgeMessage(char *message, uint8_t message_len, uint8_t pwm, uint16_t delay_ms, uint8_t bank = 0);

protected:
    void selectBank(uint8_t bank);
    void modifyRegister8(uint8_t bank, uint8_t reg, uint8_t val, uint8_t mask);
    void writeRegister8(uint8_t bank, uint8_t reg, uint8_t val);
    uint8_t readRegister8(uint8_t bank, uint8_t reg);
    uint8_t _ledDriver;
    uint8_t _i2caddr; ///< The I2C address we expect to find the chip
};