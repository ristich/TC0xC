#include "TC_IS31FL3731.h"
#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR 0x74
#define I2C_SDA 23u
#define I2C_SCL 18u
#define LED_SDB 22u

TC_IS31FL3731 ledController = TC_IS31FL3731();

void setup() {
  // put your setup code here, to run once:
  pinMode(I2C_SDA, OUTPUT);
  pinMode(I2C_SCL, OUTPUT);
  pinMode(LED_SDB, OUTPUT);
  digitalWrite(LED_SDB, HIGH);

  ledController.begin(I2C_SDA, I2C_SCL);
  Serial.begin(115200);
  Serial.println("seven o'clock");
  ledController.writeRegister8(0,0x24,255);
  ledController.writeRegister8(0,0x34,255);
  ledController.writeRegister8(0,0x44,255);
  ledController.writeRegister8(0,0x54,255);
  delay(1000);
  Serial.println("six o'clock");
  ledController.writeRegister8(0,0x25,255);
  ledController.writeRegister8(0,0x35,255);
  ledController.writeRegister8(0,0x45,255);
  ledController.writeRegister8(0,0x55,255);
  delay(1000);
  Serial.println("four o'clock");
  ledController.writeRegister8(0,0x26,255);
  ledController.writeRegister8(0,0x36,255);
  ledController.writeRegister8(0,0x46,255);
  ledController.writeRegister8(0,0x56,255);
  delay(1000);
  Serial.println("two o'clock"); 
  ledController.writeRegister8(0,0x27,255);
  ledController.writeRegister8(0,0x37,255);
  ledController.writeRegister8(0,0x47,255);
  ledController.writeRegister8(0,0x57,255);
  delay(1000);
  Serial.println("twelve o'clock");
  ledController.writeRegister8(0,0x28,255);
  ledController.writeRegister8(0,0x38,255);
  ledController.writeRegister8(0,0x48,255);
  ledController.writeRegister8(0,0x58,255);
  delay(1000);
  Serial.println("eleven o'clock");
  ledController.writeRegister8(0,0x29,255);
  ledController.writeRegister8(0,0x39,255);
  ledController.writeRegister8(0,0x49,255);
  ledController.writeRegister8(0,0x59,255);
  delay(1000);
  Serial.println("clearing");
  ledController.setAllLEDPWM(0);
  delay(1000);
  Serial.println("ring1");
  ledController.writeRegister8(0, 0x24, 255);
  ledController.writeRegister8(0, 0x25, 255);
  ledController.writeRegister8(0, 0x26, 255);
  ledController.writeRegister8(0, 0x27, 255);
  ledController.writeRegister8(0, 0x28, 255);
  ledController.writeRegister8(0, 0x29, 255);
  delay(1000);
  Serial.println("ring2");
  ledController.writeRegister8(0, 0x34, 255);
  ledController.writeRegister8(0, 0x35, 255);
  ledController.writeRegister8(0, 0x36, 255);
  ledController.writeRegister8(0, 0x37, 255);
  ledController.writeRegister8(0, 0x38, 255);
  ledController.writeRegister8(0, 0x39, 255);
  delay(1000);
    Serial.println("rin42");
  ledController.writeRegister8(0, 0x44, 255);
  ledController.writeRegister8(0, 0x45, 255);
  ledController.writeRegister8(0, 0x46, 255);
  ledController.writeRegister8(0, 0x47, 255);
  ledController.writeRegister8(0, 0x48, 255);
  ledController.writeRegister8(0, 0x49, 255);
  delay(1000);
    Serial.println("ring4");
  ledController.writeRegister8(0, 0x54, 255);
  ledController.writeRegister8(0, 0x55, 255);
  ledController.writeRegister8(0, 0x56, 255);
  ledController.writeRegister8(0, 0x57, 255);
  ledController.writeRegister8(0, 0x58, 255);
  ledController.writeRegister8(0, 0x59, 255);
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  // for (uint8_t i=0; i<500; i+=25) {
  //   ledController.setAllLEDPWM(i);
  //   delay(250);
  // }
  // for (uint8_t i=0; i<0xFF; i+=1 ) {
  //   Serial.printf("\nRegister %x:",i);
  //   ledController.setLED(i,255);
  //   delay(500);
  //   ledController.setLED(i,0);
  //   delay(500);
  // }
//  for (uint8_t i=0x24; i<=0xAB; i++){
//    ledController.writeRegister8(0,i,255);
//    delay(500);
//    Serial.printf("%x\n");
//  }
}
