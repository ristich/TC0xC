#include <Arduino.h>
#include "rtos.h"
#include "audio.h"
#include "hal.h"
#include "pitches.h"
#include "songs.h"

const song_t songs[] = {
    {foo, sizeof(foo) / sizeof(tone_t)}};

void audio_task(void *pvParameters);

Audio_Error audio_init(Audio_Object *audio)
{
    if (audio->initialized)
        return AUDIO_SUCCESS;

    xTaskCreatePinnedToCore(audio_task, "audio_task", 2048, audio, tskIDLE_PRIORITY + 2, &audio->task_handle, app_cpu);

    audio->initialized = true;

    return AUDIO_SUCCESS;
}

void audio_task(void *pvParameters)
{
    Audio_Object *audio = (Audio_Object *)pvParameters;
    uint32_t song_index = 0;
    // static bool playing = false;
    // (void)audio;

    while (1)
    {
        song_index = 0;
        if (xTaskNotifyWaitIndexed(0, 0, 0, NULL, 0) == pdTRUE)
        {
            while (song_index < songs[0].length)
            {
                // using tone's duration parameter messes with task notify
                tone(BUZZER_PIN, songs[0].notes[song_index].note);
                vTaskDelay(songs[0].notes[song_index].hold);
                song_index++;
            }
            noTone(BUZZER_PIN);
            xTaskNotifyStateClearIndexed(audio->task_handle, 0);
        }
        vTaskDelay(10);
    }
}
