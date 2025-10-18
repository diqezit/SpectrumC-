// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AudioCapture.h: Captures a single audio session and reports its status.
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
            const float* data,
            size_t samples,
            int channels
        ) = 0;
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Manages a single audio capture session.
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    class AudioCapture {
    public:
        AudioCapture();
        ~AudioCapture();

        AudioCapture(const AudioCapture&) = delete;
        AudioCapture& operator=(const AudioCapture&) = delete;
        AudioCapture(AudioCapture&&) = delete;
        AudioCapture& operator=(AudioCapture&&) = delete;

        bool Initialize();
        bool Start();
        void Stop() noexcept;

        bool IsCapturing() const noexcept;
        bool IsInitialized() const noexcept;
        bool IsFaulted() const noexcept;
        HRESULT GetLastError() const noexcept;

        void SetCallback(IAudioCaptureCallback* callback) noexcept;

        int GetSampleRate() const noexcept;
        int GetChannels() const noexcept;
        int GetBitsPerSample() const noexcept;

    private:
        struct Implementation;
        std::unique_ptr<Implementation> m_pimpl;
    };

}

#endif