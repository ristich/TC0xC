#pragma once

typedef enum Audio_Error
{
    AUDIO_SUCCESS = 0,
} Audio_Error;

// audio object to be passed around for audio control
typedef struct Audio_Object
{
    bool initialized;         // state of audio task creation
    TaskHandle_t task_handle; // task handle
} Audio_Object;

typedef enum Song_Index
{
    DOWN_BTN_SONG = 0,
    LEFT_BTN_SONG,
    UP_BTN_SONG,
    RIGHT_BTN_SONG,
    SELECT_BTN_SONG,
    FOO_SONG,
    BAR_SONG,
    TOTAL_SONGS, // should always be last
} Song_Index;

typedef struct tone_t
{
    uint16_t note;
    uint8_t hold;
} tone_t;

typedef struct song_t
{
    const tone_t *notes;
    uint32_t length;
} song_t;

Audio_Error audio_init(Audio_Object *audio);