#pragma once

#define COMMON_SAMPLE_RATE 16000
#define SDL_MAIN_HANDLED

#include "deps/SDL2/SDL.h"
#include <vector>
#include <string>
#include <cmath>
#include <cstdint>
#include <regex>
#include <atomic>
#include <mutex>
#include <cassert>
#include <cstdio>
#include <thread>

class audio_async {
public:
    audio_async() {
        m_running = false;
    }
    
    ~audio_async() {
        if (m_dev_id_in) SDL_CloseAudioDevice(m_dev_id_in);
    }

    bool init() {
        SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

        if (SDL_Init(SDL_INIT_AUDIO) < 0) return false;

        SDL_SetHintWithPriority(SDL_HINT_AUDIO_RESAMPLING_MODE, "medium", SDL_HINT_OVERRIDE);

        SDL_AudioSpec capture_spec_requested;
        SDL_AudioSpec capture_spec_obtained;

        SDL_zero(capture_spec_requested);
        SDL_zero(capture_spec_obtained);

        capture_spec_requested.freq     = COMMON_SAMPLE_RATE;
        capture_spec_requested.format   = AUDIO_F32;
        capture_spec_requested.channels = 1;
        capture_spec_requested.samples  = 1024;
        capture_spec_requested.callback = [](void * userdata, uint8_t * stream, int len) {
            audio_async * audio = (audio_async *) userdata;
            audio->callback(stream, len);
        };
        capture_spec_requested.userdata = this;

        m_dev_id_in = SDL_OpenAudioDevice(nullptr, SDL_TRUE, &capture_spec_requested, &capture_spec_obtained, 0);
    

        if (!m_dev_id_in) {
            m_dev_id_in = 0;
            return false;
        }

        m_sample_rate = capture_spec_obtained.freq;

        return true;
    }

    // start capturing audio via the provided SDL callback
    // keep last len_ms seconds of audio in a circular buffer
    bool resume() {
        if (!m_dev_id_in) return false;
        if (m_running) return false;
        SDL_PauseAudioDevice(m_dev_id_in, 0);
        m_running = true;
        return true;
    }
    
    bool pause() {
        if (!m_dev_id_in) return false;
        if (!m_running) return false;
        SDL_PauseAudioDevice(m_dev_id_in, 1);
        m_running = false;
        return true;
    }

    bool clear() {
        if (!m_dev_id_in) return false;
        if (!m_running) return false;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_audio_pos = 0;
        m_audio_len = 0;
        return true;
    }

    // callback to be called by SDL
    void callback(uint8_t * stream, int len) {
        if (!m_running) return;

        const size_t n_samples = len / sizeof(float);

        m_audio_new.resize(n_samples);
        memcpy(m_audio_new.data(), stream, n_samples * sizeof(float));

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (m_audio_pos + n_samples > m_audio.size()) {
                const size_t n0 = m_audio.size() - m_audio_pos;

                memcpy(&m_audio[m_audio_pos], stream, n0 * sizeof(float));
                memcpy(&m_audio[0], &stream[n0], (n_samples - n0) * sizeof(float));

                m_audio_pos = (m_audio_pos + n_samples) % m_audio.size();
                m_audio_len = m_audio.size();
            } 
            else {
                memcpy(&m_audio[m_audio_pos], stream, n_samples * sizeof(float));

                m_audio_pos = (m_audio_pos + n_samples) % m_audio.size();
                m_audio_len = std::min(m_audio_len + n_samples, m_audio.size());
            }
        }
    }

    // get audio data from the circular buffer
    void get(std::vector<float> & result) {
        if (!m_dev_id_in) return;
        if (!m_running) return;

        result.clear();

        {
            std::lock_guard<std::mutex> lock(m_mutex);

            size_t n_samples = (m_sample_rate);
            if (n_samples > m_audio_len) n_samples = m_audio_len;
            
            result.resize(n_samples);

            int s0 = m_audio_pos - n_samples;
            if (s0 < 0) s0 += m_audio.size();
              
            if (s0 + n_samples > m_audio.size()) {
                const size_t n0 = m_audio.size() - s0;
                memcpy(result.data(), &m_audio[s0], n0 * sizeof(float));
                memcpy(&result[n0], &m_audio[0], (n_samples - n0) * sizeof(float));
            } 
            else memcpy(result.data(), &m_audio[s0], n_samples * sizeof(float));
        }
    }

private:
    SDL_AudioDeviceID m_dev_id_in = 0;

    int m_sample_rate = 0;

    std::atomic_bool m_running;
    std::mutex       m_mutex;

    std::vector<float> m_audio;
    std::vector<float> m_audio_new;
    size_t             m_audio_pos = 0;
    size_t             m_audio_len = 0;
};