#include "TC_IS31FL3731.h"
#include <Arduino.h>
#include <Wire.h>
#include <IRrecv.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

// LED Configs
#define I2C_ADDR 0x74
#define I2C_SDA 23u
#define I2C_SCL 18u
#define LED_SDB 22u
TC_IS31FL3731 ledController = TC_IS31FL3731();

//IR configs
const uint16_t kRecvPin = 25;
const uint16_t kCaptureBufferSize = 1024;
const uint8_t kTolerancePercentage = kTolerance; 
const uint8_t kTimeout = 15;
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results; 



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
  irrecv.setTolerance(kTolerancePercentage);  // Override the default tolerance.
  irrecv.enableIRIn();
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
// Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    // Display a crude timestamp.
    uint32_t now = millis();
    Serial.printf(D_STR_TIMESTAMP " : %06u.%03u\n", now / 1000, now % 1000);
    // Check if we got an IR message that was to big for our capture buffer.
    if (results.overflow)
      Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    // Display the library version the message was captured with.
    Serial.println(D_STR_LIBRARY "   : v" _IRREMOTEESP8266_VERSION_STR "\n");
    // Display the tolerance percentage if it has been change from the default.
    if (kTolerancePercentage != kTolerance)
      Serial.printf(D_STR_TOLERANCE " : %d%%\n", kTolerancePercentage);
    // Display the basic output of what we found.
    Serial.print(resultToHumanReadableBasic(&results));
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();  // Feed the WDT as the text output can take a while to print.
#if LEGACY_TIMING_INFO
    // Output legacy RAW timing info of the result.
    Serial.println(resultToTimingInfo(&results));
    yield();  // Feed the WDT (again)
#endif  // LEGACY_TIMING_INFO
    // Output the results as source code
    Serial.println(resultToSourceCode(&results));
    Serial.println();    // Blank line between entries
    yield();             // Feed the WDT (again)
  }
}
