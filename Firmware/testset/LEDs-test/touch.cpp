#include "arduino.h"
#include "touch.h"

// simple debounce check for touch sensing
//   gpioPin: gpio or touch number
//   threshold: threshold for touchRead
// uint8_t checkTouch(uint8_t gpioPin, uint8_t threshold)
// {
//   if (touchRead(gpioPin) <= threshold) {
//     if (touchRead(gpioPin) <= threshold) {
//       if (touchRead(gpioPin) <= threshold) {
//         return 1;
//       }
//     }
//   }

//   return 0;
// }

bool touchInterrupt()
{
  if ((checkTouch(DOWN_TOUCH,  TOUCH_THRESH)) || (checkTouch(LEFT_TOUCH,   TOUCH_THRESH)) || 
      (checkTouch(UP_TOUCH,    TOUCH_THRESH)) || (checkTouch(RIGHT_TOUCH,  TOUCH_THRESH)) || 
      (checkTouch(START_TOUCH, TOUCH_THRESH)) || (checkTouch(A_TOUCH,      TOUCH_THRESH)) || 
      (checkTouch(B_TOUCH,     TOUCH_THRESH)) || (checkTouch(SELECT_TOUCH, TOUCH_THRESH)))
  {
    return true;
  }

  return false;
}
