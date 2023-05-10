#pragma once

#define COMMON_SAMPLE_RATE 16000
#define SDL_MAIN_HANDLED

#include "deps/SDL2/SDL.h"
#include <vector>
#include <regex>
#include <atomic>
#include <thread>

//Maximum recording time
const int MAX_RECORDING_SECONDS = 19;
const int RECORDING_BUFFER_SECONDS = MAX_RECORDING_SECONDS + 1;

class audio_async {
public:
    audio_async() {
        m_running = false;
    }
    
    ~audio_async() {
        if (m_dev_id_in) SDL_CloseAudioDevice(m_dev_id_in);
        free(gRecordingBuffer);
    }

    bool init() {
        // Sets audio events to higher prio
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
        if (SDL_Init(SDL_INIT_AUDIO) < 0) return false;
        SDL_SetHintWithPriority(SDL_HINT_AUDIO_RESAMPLING_MODE, "medium", SDL_HINT_OVERRIDE);

        // Defines audio input device
        SDL_AudioSpec capture_spec_requested;
        SDL_AudioSpec capture_spec_obtained;
        SDL_zero(capture_spec_requested);
        SDL_zero(capture_spec_obtained);
        capture_spec_requested.freq     = COMMON_SAMPLE_RATE;
        capture_spec_requested.format   = AUDIO_F32;
        capture_spec_requested.channels = 1;
        capture_spec_requested.samples  = 4096;
        capture_spec_requested.callback = [](void * userdata, uint8_t * stream, int len) {
            audio_async * audio = (audio_async *) userdata;
            audio->callback(stream, len);
        };
        capture_spec_requested.userdata = this;
        m_dev_id_in = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &capture_spec_requested, &capture_spec_obtained, 0);
        if (!m_dev_id_in) return false;

        // Calculates buffer size
        int bytesPerSample = capture_spec_obtained.channels * (SDL_AUDIO_BITSIZE(capture_spec_obtained.format) / 8);
	    int bytesPerSecond = capture_spec_obtained.freq * bytesPerSample;
	    gBufferByteSize = RECORDING_BUFFER_SECONDS * bytesPerSecond;
        if ((gBufferByteSize % sizeof(float)) != 0) return false; // Dummy proof

	    // Allocate and initialize byte buffer
	    gRecordingBuffer = new Uint8[gBufferByteSize];

        return true;
    }

    bool start() {
        if (!m_dev_id_in) return false;
        if (m_running) return false;
        std::memset(gRecordingBuffer, 0, gBufferByteSize); // Clears mem prior to reuse
        SDL_PauseAudioDevice(m_dev_id_in, 0);
        m_running = true;
        return true;
    }
    
    bool end(std::vector<float> & result) {
        if (!m_dev_id_in) return false;
        if (!m_running) return false;
        SDL_PauseAudioDevice(m_dev_id_in, 1);
        m_running = false;
        result.resize(gBufferByteSize / sizeof(float));
        std::memcpy(result.data(), (float*)gRecordingBuffer, gBufferByteSize);
        gBufferBytePosition = 0;
        return true;
    }

    // Called by SDL, records audio
    void callback(Uint8* stream, int len ) {
        if (!m_running) return;
        if (gBufferBytePosition + len > gBufferByteSize) return;
	    std::memcpy(&gRecordingBuffer[ gBufferBytePosition ], stream, len);
	    gBufferBytePosition += len;
    }

private:
    // Defines recording buffer
    Uint8* gRecordingBuffer = NULL;
    Uint32 gBufferByteSize = 0;
    Uint32 gBufferBytePosition = 0;
    
    SDL_AudioDeviceID m_dev_id_in = 0;
    std::atomic_bool m_running;
};