#ifndef _TOUCH_H_
#define _TOUCH_H_

// touch button defines
#define DOWN_TOUCH    12
#define LEFT_TOUCH    2
#define UP_TOUCH      15
#define RIGHT_TOUCH   13
#define SELECT_TOUCH  14
#define START_TOUCH   4
#define A_TOUCH       T7
#define B_TOUCH       0

// touch button tresholds
#define TOUCH_THRESH  25

uint8_t checkTouch(uint8_t gpioPin, uint8_t threshold);
bool touchInterrupt();

#endif // _TOUCH_H_