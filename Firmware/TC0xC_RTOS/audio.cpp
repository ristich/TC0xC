#include <Arduino.h>
#include "rtos.h"
#include "audio.h"
#include "hal.h"
#include "pitches.h"
#include "songs.h"

const song_t songs[] = {
    // it's important that button songs be at the start in this order
    {button_down, sizeof(button_down) / sizeof(tone_t)},
    {button_left, sizeof(button_left) / sizeof(tone_t)},
    {button_up, sizeof(button_up) / sizeof(tone_t)},
    {button_right, sizeof(button_right) / sizeof(tone_t)},
    {button_select, sizeof(button_select) / sizeof(tone_t)},
    {stage_complete, sizeof(stage_complete) / sizeof(tone_t)},
    {rick, sizeof(rick) / sizeof(tone_t)}};

void audio_task(void *pvParameters);
Song_Error play_song(uint32_t *index);

Audio_Error audio_init(Audio_Object *audio)
{
    if (audio->initialized)
        return AUDIO_SUCCESS;

    pinMode(BUZZER_PIN, OUTPUT);

    ledcSetup(BUZZER_CHANNEL, BUZZER_DEFAULT_FREQ, BUZZER_RESOLUTION);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
    ledcWriteTone(BUZZER_CHANNEL, 0);

    xTaskCreatePinnedToCore(audio_task, "audio_task", 2048, audio, tskIDLE_PRIORITY + 2, &audio->task_handle, app_cpu);

    audio->initialized = true;

    return AUDIO_SUCCESS;
}

void audio_task(void *pvParameters)
{
    // Audio_Object *audio = (Audio_Object *)pvParameters;
    uint32_t song_index;
    uint32_t *pSong_index = &song_index;

    while (1)
    {
        song_index = TOTAL_SONGS;
        if (xTaskNotifyWaitIndexed(0, 0, 0, pSong_index, 0) == pdTRUE)
        {
            song_index = RICK_SONG;
            while(play_song(pSong_index) == SONG_INTERRUPTED);
        }
        vTaskDelay(10);
    }
}

Song_Error play_song(uint32_t *index)
{
    if (*index >= TOTAL_SONGS)
        return SONG_INDEX_TOO_HIGH;

    uint32_t note_index = 0;
    song_t const *song = &songs[*index];
    while (note_index < song->length)
    {
        ledcWriteTone(BUZZER_CHANNEL, song->notes[note_index].note);
        if (xTaskNotifyWaitIndexed(0, 0, 0, index, song->notes[note_index].hold) == pdTRUE)
        {
            ledcWriteTone(BUZZER_CHANNEL, 0);
            return SONG_INTERRUPTED;
        }
        note_index++;
    }
    
    ledcWriteTone(BUZZER_CHANNEL, 0);
    return SONG_COMPLETE;
}
