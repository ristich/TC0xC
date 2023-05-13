#include "TC_IS31FL3731.h"

// badge LEDs
uint8_t LEDRegAddr[4] = {0x00, 0x02, 0x04, 0x06};
uint8_t LEDRegUsed = 0x3F;

/**************************************************************************/
/*!
    @brief Initialize hardware and clear display
    @param addr The I2C address we expect to find the chip at
    @returns True on success, false if chip isnt found
*/
/**************************************************************************/
bool TC_IS31FL3731::begin(uint8_t sdaPin, uint8_t sclPin, uint8_t addr)
{
    Wire.begin(sdaPin, sclPin, 400000);

    _i2caddr = addr;

    // A basic scanner, see if it ACK's
    Wire.beginTransmission(_i2caddr);
    if (Wire.endTransmission() != 0)
    {
        return false;
    }

    // shutdown
    writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x00);

    delay(10);

    // out of shutdown
    writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_SHUTDOWN, 0x01);

    // picture mode
    writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_CONFIG,
                   CONFIG_MODE_PICTURE);
    setPictureFrame(0);

    for (int i = 0; i < 8; i++)
    {
        // set PWM value to 0 for each LED
        setAllLEDPWM(0, i);

        // turn on only the LEDs we have available
        for (uint8_t j = 0; j < 4; j++)
        {
            writeRegister8(i, LEDRegAddr[j], LEDRegUsed);
        }
    }

    audioSync(false);

    return true;
}

/**************************************************************************/
/*!
    @brief Sets all LEDs on & 0 PWM for current frame.
    @note It's recommended to just adjust PWM values with setAllLEDPWM
          instead of using this function to avoid messing with LED state
    @param bank The bank/frame we will set the data in
*/
/**************************************************************************/
void TC_IS31FL3731::clear(uint8_t bank)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        Wire.beginTransmission(_i2caddr);
        Wire.write((byte)0x24 + i * 16);
        // write 24 bytes at once
        for (uint8_t j = 0; j < 6; j++)
        {
            Wire.write((byte)0);
        }
        Wire.endTransmission();
    }
}

/**************************************************************************/
/*!
    @brief Low level accesssor - sets a 8-bit PWM pixel value to a bank location
    does not handle rotation, x/y or any rearrangements!
    @param led The offset into the bank that corresponds to the LED
    @param pwm brightnes, from 0 (off) to 255 (max on)
    @param bank The bank/frame we will set the data in
*/
/**************************************************************************/
void TC_IS31FL3731::setLEDPWM(uint8_t led, uint8_t pwm, uint8_t bank)
{
    if (led >= 144)
        return;
    writeRegister8(bank, 0x24 + led, pwm);
}

/**************************************************************************/
/*!
    @brief Low level accesssor - sets a 8-bit PWM pixel value to a bank location
    does not handle rotation, x/y or any rearrangements!
    @param pwm brightnes, from 0 (off) to 255 (max on)
    @param bank The bank/frame we will set the data in
*/
/**************************************************************************/
void TC_IS31FL3731::setAllLEDPWM(uint8_t pwm, uint8_t bank)
{
    selectBank(bank);

    for (uint8_t i = 0; i < 4; i++)
    {
        Wire.beginTransmission(_i2caddr);
        Wire.write((byte)0x24 + i * 16);
        // write 6 bytes at once
        for (uint8_t j = 0; j < 6; j++)
        {
            Wire.write((byte)pwm);
        }
        Wire.endTransmission();
    }
}

void TC_IS31FL3731::setColumn(led_col_t col, uint8_t pwm, uint8_t bank)
{
    switch (col)
    {
    case LED_COL_SEVEN:
        writeRegister8(bank, 0x24, pwm);
        writeRegister8(bank, 0x34, pwm);
        writeRegister8(bank, 0x44, pwm);
        writeRegister8(bank, 0x54, pwm);
        break;

    case LED_COL_SIX:
        writeRegister8(bank, 0x25, pwm);
        writeRegister8(bank, 0x35, pwm);
        writeRegister8(bank, 0x45, pwm);
        writeRegister8(bank, 0x55, pwm);
        break;

    case LED_COL_FIVE:
        writeRegister8(bank, 0x26, pwm);
        writeRegister8(bank, 0x36, pwm);
        writeRegister8(bank, 0x46, pwm);
        writeRegister8(bank, 0x56, pwm);
        break;

    case LED_COL_ONE:
        writeRegister8(bank, 0x27, pwm);
        writeRegister8(bank, 0x37, pwm);
        writeRegister8(bank, 0x47, pwm);
        writeRegister8(bank, 0x57, pwm);
        break;

    case LED_COL_TWELVE:
        writeRegister8(bank, 0x28, pwm);
        writeRegister8(bank, 0x38, pwm);
        writeRegister8(bank, 0x48, pwm);
        writeRegister8(bank, 0x58, pwm);
        break;

    case LED_COL_ELEVEN:
        writeRegister8(bank, 0x29, pwm);
        writeRegister8(bank, 0x39, pwm);
        writeRegister8(bank, 0x49, pwm);
        writeRegister8(bank, 0x59, pwm);
        break;

    default:
        break;
    }
}

/**************************************************************************/
/*!
    @brief Sets the LED intensity/PWM for the single color LEDs
    @param lednum Badge LED to set [0 - 23]
    @param pwm PWM value for LED from 0 (off) to 255 (max on)
    @param bank The bank/frame we will set the data in
*/
/**************************************************************************/
void TC_IS31FL3731::setBadgeLED(uint8_t lednum, uint8_t pwm, uint8_t bank)
{
    if (lednum > 23)
        return;

    selectBank(bank);

    Wire.beginTransmission(_i2caddr);
    Wire.write(led_addrs[lednum]);
    Wire.write((byte)pwm);
    Wire.endTransmission();
}

/**
 * @brief Set all Badge leds at once
 *
 * @param config leds to set
 * @param pwm pwm value to set to
 * @param bank  the bank/frame to set the data in
 */
void TC_IS31FL3731::setBadgeLEDs(uint32_t config, uint8_t pwm, uint8_t bank)
{
    selectBank(bank);

    // light up a through x
    for (uint8_t i = 0; i < 24; i++)
    {
        uint8_t led_set = (config >> i) & 0x1;
        if (led_set)
        {
            Wire.beginTransmission(_i2caddr);
            Wire.write((byte)led_addrs[i]);
            Wire.write((byte)pwm);
            Wire.endTransmission();
        }
    }
}

void TC_IS31FL3731::setBadgeLetter(char letter, uint8_t pwm, uint8_t bank)
{
    if (letter >= 'a' && letter <= 'x')
    {
        uint8_t config = letter - 'a';
        setBadgeLED(config, pwm, bank);
    }
    else if (letter == 'y')
    {
        uint32_t config = 0x800001;
        setBadgeLEDs(config, pwm, bank);
    }
    else if (letter == 'z')
    {
        uint32_t config = 0x800002;
        setBadgeLEDs(config, pwm, bank);
    }
    else if (letter == ' ')
    {
        uint32_t config = 0xFFFFFF;
        setBadgeLEDs(config, pwm, bank);
    }
}

void TC_IS31FL3731::setBadgeMessage(char *message, uint8_t message_len, uint8_t pwm, uint16_t delay_ms, uint8_t bank)
{
    for (uint8_t i = 0; i < message_len; i++)
    {
        // check for string end
        if (message[i] == 0)
            return;
        
        clear();
        vTaskDelay(10);
        setBadgeLetter(message[i], pwm, bank);
        vTaskDelay(delay_ms);
    }
}

/**************************************************************************/
/*!
    @brief Sets the display more of the LED driver
    @param mode Mode to change to; picture (single frame), auto (mutliple
                frames in progression), audio
*/
/**************************************************************************/
void TC_IS31FL3731::setDisplayMode(Display_Mode mode)
{
    uint8_t newMode = CONFIG_MODE_PICTURE;

    if (mode == Display_Mode_Auto_Play)
        newMode = CONFIG_MODE_AUTOPLAY;
    else if (mode == Display_Mode_Audio_Play)
        newMode = CONFIG_MODE_AUDIOPLAY;

    modifyRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_CONFIG, newMode,
                    CONFIG_MODE_MASK);

    return;
}

/**************************************************************************/
/*!
    @brief Sets starting frame for auto play
    @param frame Frame (0-7)
*/
/**************************************************************************/
void TC_IS31FL3731::setAutoPlayStart(uint8_t frame)
{
    if (frame > 7)
    {
        frame = 0;
    }

    modifyRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_CONFIG, frame,
                    CONFIG_FRAME_MASK);

    return;
}

/**************************************************************************/
/*!
    @brief Sets number of loops for auto play
    @param loops Number of loops to play through (1-7) or 0 for unlimited
*/
/**************************************************************************/
void TC_IS31FL3731::setAutoPlayLoops(uint8_t loops)
{
    if (loops > 7)
    {
        loops = 0;
    }
    uint8_t newVal = loops << AUTO_CONTROL_LOOPS_SHIFT;

    modifyRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_AUTO_CONTROL1, newVal,
                    AUTO_CONTROL_LOOPS_MASK);

    return;
}

/**************************************************************************/
/*!
    @brief Sets number of frames to play through per loop in auto play
    @param frames Number of frames to play through (1-7) or 0 for all
*/
/**************************************************************************/
void TC_IS31FL3731::setAutoPlayFrames(uint8_t frames)
{
    if (frames > 7)
    {
        frames = 0;
    }

    modifyRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_AUTO_CONTROL1, frames,
                    AUTO_CONTROL_FRAMES_MASK);

    return;
}

/**************************************************************************/
/*!
    @brief Sets time between frame transitions for auto play
    @param delay_ms Delay in milliseconds (max 693 ms)
*/
/**************************************************************************/
void TC_IS31FL3731::setAutoPlayDelay(uint16_t delay_ms)
{
    if (delay_ms > 693)
    {
        delay_ms = 693;
    }
    uint8_t newVal = (delay_ms / 11);

    writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_AUTO_CONTROL2, newVal);

    return;
}

/**************************************************************************/
/*!
    @brief Have the chip set the display to the contents of a frame
    @param frame Ranges from 0 - 7 for the 8 frames
*/
/**************************************************************************/
void TC_IS31FL3731::setPictureFrame(uint8_t frame)
{
    if (frame > 7)
    {
        frame = 0;
    }

    writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTURE_DISPLAY, frame);
}

/**************************************************************************/
/*!
    @brief Switch to a given bank in the chip memory for future reads
    @param bank The IS31 bank to switch to
*/
/**************************************************************************/
void TC_IS31FL3731::selectBank(uint8_t bank)
{
    Wire.beginTransmission(_i2caddr);
    Wire.write((byte)ISSI_COMMANDREGISTER);
    Wire.write(bank);
    Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief Enable the audio 'sync' for brightness pulsing (not really tested)
    @param sync True to enable, False to disable
*/
/**************************************************************************/
void TC_IS31FL3731::audioSync(bool sync)
{
    if (sync)
    {
        writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_AUDIOSYNC, 0x1);
    }
    else
    {
        writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_AUDIOSYNC, 0x0);
    }
}

/**************************************************************************/
/*!
    @brief Modify one byte to a register located in a given bank
    @param bank The IS31 bank to write the register location
    @param reg the offset into the bank to write
    @param val The byte value
    @param mask Modify only the bits that are set to one
*/
/**************************************************************************/
void TC_IS31FL3731::modifyRegister8(uint8_t bank, uint8_t reg, uint8_t val,
                                    uint8_t mask)
{
    uint8_t currReg = readRegister8(bank, reg);
    currReg &= ~(mask);
    val |= currReg;
    writeRegister8(bank, reg, val);
}

/**************************************************************************/
/*!
    @brief Write one byte to a register located in a given bank
    @param bank The IS31 bank to write the register location
    @param reg the offset into the bank to write
    @param val The byte value
*/
/**************************************************************************/
void TC_IS31FL3731::writeRegister8(uint8_t bank, uint8_t reg, uint8_t val)
{
    selectBank(bank);

    Wire.beginTransmission(_i2caddr);
    Wire.write((byte)reg);
    Wire.write((byte)val);
    Wire.endTransmission();
}

/**************************************************************************/
/*!
    @brief  Read one byte from a register located in a given bank
    @param   bank The IS31 bank to read the register location
    @param   reg the offset into the bank to read
    @return 1 byte value
*/
/**************************************************************************/
uint8_t TC_IS31FL3731::readRegister8(uint8_t bank, uint8_t reg)
{
    uint8_t x;

    selectBank(bank);

    Wire.beginTransmission(_i2caddr);
    Wire.write((byte)reg);
    Wire.endTransmission();

    Wire.requestFrom(_i2caddr, (size_t)1);
    x = Wire.read();

    return x;
}
