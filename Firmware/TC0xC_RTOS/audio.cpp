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
    {foo, sizeof(foo) / sizeof(tone_t)},
    {bar, sizeof(bar) / sizeof(tone_t)}};

void audio_task(void *pvParameters);
void play_song(uint32_t index);

Audio_Error audio_init(Audio_Object *audio)
{
    if (audio->initialized)
        return AUDIO_SUCCESS;

    tone(BUZZER_PIN, 0);

    xTaskCreatePinnedToCore(audio_task, "audio_task", 2048, audio, tskIDLE_PRIORITY + 2, &audio->task_handle, app_cpu);

    audio->initialized = true;

    return AUDIO_SUCCESS;
}

void audio_task(void *pvParameters)
{
    Audio_Object *audio = (Audio_Object *)pvParameters;
    uint32_t song_index;
    uint32_t *pSong_index = &song_index;

    while (1)
    {
        song_index = 0;
        if (xTaskNotifyWaitIndexed(0, 0, 0, pSong_index, 0) == pdTRUE)
        {
            play_song(song_index);
            xTaskNotifyStateClearIndexed(audio->task_handle, 0);
        }
        vTaskDelay(10);
    }
}

void play_song(uint32_t index)
{
    if (index >= TOTAL_SONGS)
        return;

    uint32_t note_index = 0;
    song_t const *song = &songs[index];
    while (note_index < song->length)
    {
        // using tone's duration parameter messes with task notify
        tone(BUZZER_PIN, song->notes[note_index].note);
        vTaskDelay(song->notes[note_index].hold);
        note_index++;
    }
    noTone(BUZZER_PIN);
}
