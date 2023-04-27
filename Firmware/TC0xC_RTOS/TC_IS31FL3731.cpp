#include "TC_IS31FL3731.h"

// badge LEDs
uint8_t LEDRegAddr[7] = {0x00, 0x02, 0x04, 0x06};
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
  _frame = 0;

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
                 ISSI_REG_CONFIG_PICTUREMODE);

  displayFrame(_frame);

  // all LEDs on & 0 PWM
  clear(); // set each led to 0 PWM

  for (uint8_t i = 0; i < 4; i++)
  {
    writeRegister8(0, LEDRegAddr[i], LEDRegUsed);
  }

  audioSync(false);

  return true;
}

/**************************************************************************/
/*!
    @brief Sets all LEDs on & 0 PWM for current frame.
*/
/**************************************************************************/
void TC_IS31FL3731::clear(void)
{
  selectBank(_frame);

  for (uint8_t i = 0; i < 6; i++)
  {
    Wire.beginTransmission(_i2caddr);
    Wire.write((byte)0x24 + i * 24);
    // write 24 bytes at once
    for (uint8_t j = 0; j < 24; j++)
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
    @param lednum The offset into the bank that corresponds to the LED
    @param bank The bank/frame we will set the data in
    @param pwm brightnes, from 0 (off) to 255 (max on)
*/
/**************************************************************************/
void TC_IS31FL3731::setLEDPWM(uint8_t lednum, uint8_t pwm, uint8_t bank)
{
  if (lednum >= 144)
    return;
  writeRegister8(bank, 0x24 + lednum, pwm);
}

/**************************************************************************/
/*!
    @brief Low level accesssor - sets a 8-bit PWM pixel value to a bank location
    does not handle rotation, x/y or any rearrangements!
    @param lednum The offset into the bank that corresponds to the LED
    @param bank The bank/frame we will set the data in
    @param pwm brightnes, from 0 (off) to 255 (max on)
*/
/**************************************************************************/
void TC_IS31FL3731::setAllLEDPWM(uint8_t pwm, uint8_t bank)
{
  selectBank(_frame);

  for (uint8_t i = 0; i < 6; i++)
  {
    Wire.beginTransmission(_i2caddr);
    Wire.write((byte)0x24 + i * 24);
    // write 24 bytes at once
    for (uint8_t j = 0; j < 24; j++)
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
    writeRegister8(0, 0x24, pwm);
    writeRegister8(0, 0x34, pwm);
    writeRegister8(0, 0x44, pwm);
    writeRegister8(0, 0x54, pwm);
    break;

  case LED_COL_SIX:
    writeRegister8(0, 0x25, pwm);
    writeRegister8(0, 0x35, pwm);
    writeRegister8(0, 0x45, pwm);
    writeRegister8(0, 0x55, pwm);
    break;

  case LED_COL_FIVE:
    writeRegister8(0, 0x26, pwm);
    writeRegister8(0, 0x36, pwm);
    writeRegister8(0, 0x46, pwm);
    writeRegister8(0, 0x56, pwm);
    break;

  case LED_COL_ONE:
    writeRegister8(0, 0x27, pwm);
    writeRegister8(0, 0x37, pwm);
    writeRegister8(0, 0x47, pwm);
    writeRegister8(0, 0x57, pwm);
    break;

  case LED_COL_TWELVE:
    writeRegister8(0, 0x28, pwm);
    writeRegister8(0, 0x38, pwm);
    writeRegister8(0, 0x48, pwm);
    writeRegister8(0, 0x58, pwm);
    break;

  case LED_COL_ELEVEN:
    writeRegister8(0, 0x29, pwm);
    writeRegister8(0, 0x39, pwm);
    writeRegister8(0, 0x49, pwm);
    writeRegister8(0, 0x59, pwm);
    break;

  default:
    break;
  }
}

// this assumes LEDs are already on
// v

void TC_IS31FL3731::setLED(uint8_t led, uint8_t pwm, uint8_t bank)
{
  switch (led)
  {
  case UP_LED:
    writeRegister8(bank, 0x68, pwm);
    break;

  case RIGHT_LED:
    writeRegister8(bank, 0x78, pwm);
    break;

  case DOWN_LED:
    writeRegister8(bank, 0x57, pwm);
    break;

  case LEFT_LED:
    writeRegister8(bank, 0x67, pwm);
    break;

  case SELECT_LED:
    writeRegister8(bank, 0x8A, pwm);
    break;

  case START_LED:
    writeRegister8(bank, 0x9A, pwm);
    break;

  case B_LED:
    writeRegister8(bank, 0x9B, pwm);
    break;

  case A_LED:
    writeRegister8(bank, 0xAB, pwm);
    break;

  // TODO: set the correct registers
  case SEAT1_LED:
    writeRegister8(bank, 0x35, pwm);
    break;

  case SEAT2_LED:
    writeRegister8(bank, 0x45, pwm);
    break;

  case SEAT3_LED:
    writeRegister8(bank, 0x24, pwm);
    break;

  case SEAT4_LED:
    writeRegister8(bank, 0x34, pwm);
    break;

  case SEAT5_LED:
    writeRegister8(bank, 0x25, pwm);
    break;

  default:
    break;
  }
}

/**************************************************************************/
/*!
    @brief Set's this object's frame tracker (does not talk to the chip)
    @param frame Ranges from 0 - 7 for the 8 frames
*/
/**************************************************************************/
void TC_IS31FL3731::setFrame(uint8_t frame) { _frame = frame; }

/**************************************************************************/
/*!
    @brief Have the chip set the display to the contents of a frame
    @param frame Ranges from 0 - 7 for the 8 frames
*/
/**************************************************************************/
void TC_IS31FL3731::displayFrame(uint8_t frame)
{
  if (frame > 7)
    frame = 0;
  writeRegister8(ISSI_BANK_FUNCTIONREG, ISSI_REG_PICTUREFRAME, frame);
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
    @brief Write one byte to a register located in a given bank
    @param bank The IS31 bank to write the register location
    @param reg the offset into the bank to write
    @param data The byte value
*/
/**************************************************************************/
void TC_IS31FL3731::writeRegister8(uint8_t bank, uint8_t reg,
                                   uint8_t data)
{
  selectBank(bank);

  Wire.beginTransmission(_i2caddr);
  Wire.write((byte)reg);
  Wire.write((byte)data);
  Wire.endTransmission();
  // Serial.print("$"); Serial.print(reg, HEX);
  // Serial.print(" = 0x"); Serial.println(data, HEX);
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

  // Serial.print("$"); Serial.print(reg, HEX);
  // Serial.print(": 0x"); Serial.println(x, HEX);

  return x;
}
