// AudioCapture.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AudioCapture.h: Captures audio from the default playback device (loopback).
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef SPECTRUM_CPP_AUDIO_CAPTURE_H
#define SPECTRUM_CPP_AUDIO_CAPTURE_H

#include "Common.h"
#include <memory>

namespace Spectrum {

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Interface for receiving audio data callbacks.
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class IAudioCaptureCallback {
    public:
        virtual ~IAudioCaptureCallback() = default;
        virtual void OnAudioData(
            const float* data, size_t samples, int channels
        ) = 0;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Main audio capture class, providing a simple public API.
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class AudioCapture {
    public:
        AudioCapture();
        ~AudioCapture();

        AudioCapture(const AudioCapture&) = delete;
        AudioCapture& operator=(const AudioCapture&) = delete;
        AudioCapture(AudioCapture&&) = delete;
        AudioCapture& operator=(AudioCapture&&) = delete;

        // Lifecycle management
        bool Initialize();
        bool Start();
        void Stop() noexcept;

        // State getters
        bool IsCapturing() const noexcept;
        bool IsInitialized() const noexcept;

        // Configuration
        void SetCallback(IAudioCaptureCallback* callback) noexcept;

        // Audio format info
        int GetSampleRate() const noexcept;
        int GetChannels() const noexcept;
        int GetBitsPerSample() const noexcept;

    private:
        struct Implementation;
        std::unique_ptr<Implementation> m_pimpl;
    };

} // namespace Spectrum

#endif // SPECTRUM_CPP_AUDIO_CAPTURE_H