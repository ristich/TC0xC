#pragma once

#include "pitches.h"
#include "audio.h"

const tone_t silence[] = {
    {0, 0},
};

const tone_t button_down[] = {
    {NOTE_C6, 200},
};

const tone_t button_left[] = {
    {NOTE_D6, 200},
};

const tone_t button_up[] = {
    {NOTE_E6, 200},
};

const tone_t button_right[] = {
    {NOTE_F6, 200},
};

const tone_t button_select[] = {
    {NOTE_G6, 200},
};

const tone_t stage_complete[] = {
    {659,105},
    {554,105},
    {659,110},
    {554,110},
    {659,110},
    {554,110},
    {659,110},
    {554,110},
    {740,110},
    {622,110},
    {622,110},
    {740,110},
    {622,110},
    {740,110},
    {740,110},
    {622,110},
    {587,110},
    {494,110},
    {587,110},
    {494,110},
    {494,110},
    {587,110},
    {494,110},
    {587,110},
    {330,110},
    {440,110},
    {494,110},
    {349,110},
    {587,110},
    {440,110},
    {659,1713},
};

const tone_t rick[] = {
  {NOTE_C5,180},
  {NOTE_D5,180},
  {NOTE_F5,180},
  {NOTE_D5,180},
  {NOTE_A5,540},
  {NOTE_A5,540},
  {NOTE_G5,1080},
  {NOTE_C5,180},
  {NOTE_D5,180},
  {NOTE_F5,180},
  {NOTE_D5,180},
  {NOTE_G5,540},
  {NOTE_G5,540},
  {NOTE_F5,540},
  {NOTE_E5,180},
  {NOTE_D5,360},
  {NOTE_C5,180},
  {NOTE_D5,180},
  {NOTE_F5,180},
  {NOTE_D5,180},
  {NOTE_F5,720},
  {NOTE_G5,360},
  {NOTE_E5,540},
  {NOTE_D5,180},
  {NOTE_C5,360},
  {NOTE_C5,360},
  {NOTE_C5,360},
  {NOTE_G5,720},
  {NOTE_F5,1440},
  {NOTE_C5,180},
  {NOTE_D5,180},
  {NOTE_F5,180},
  {NOTE_D5,180},
  {NOTE_A5,540},
  {NOTE_A5,540},
  {NOTE_G5,1080},
  {NOTE_C5,180},
  {NOTE_D5,180},
  {NOTE_F5,180},
  {NOTE_D5,180},
  {NOTE_C6,720},
  {NOTE_E5,360},
  {NOTE_F5,540},
  {NOTE_E5,180},
  {NOTE_D5,360},
  {NOTE_C5,180},
  {NOTE_D5,180},
  {NOTE_F5,180},
  {NOTE_D5,180},
  {NOTE_F5,720},
  {NOTE_G5,360},
  {NOTE_E5,540},
  {NOTE_D5,180},
  {NOTE_C5,720},
  {NOTE_C5,360},
  {NOTE_G5,720},
  {NOTE_F5,1080}
};

const tone_t hit[] = {
  {370,35},

  {466,23},

  {587,23},

  {698,35},

  {1397,163},

  {1661,35},

  {1760,105},

  {1976,151},

  {2093,116},

  {262,23},

  {988,35},

  {1175,70},

  {1245,35},

  {1568,58},

  {1865,140},

  {2217,128},

  {2637,128},

  {494,35},

  {622,35},

  {784,35},

  {2489,105},

  {2960,58},

  {659,35},

  {932,70},

  {1109,47},

  {1319,58},

  {1480,35},

  {1661,174},

  {44,23},

  {233,35},

  {311,35},

  {554,93},

  {110,23},

  {415,47},

  {988,47},

  {466,35},

  {1568,47},

  {1047,58},

  {2349,35},

  {1245,35},

  {831,35},

  {1319,93},

  {139,47},

  {784,81},

  {932,35},

  {1480,70},

  {370,35},

  {622,58},

  {1175,58},

  {1109,58},

  {3136,35},

  {247,35},

  {466,47},

  {1568,35},

  {1865,35},

  {1760,93},

  {1865,47},
};
