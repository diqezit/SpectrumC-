// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AudioManager.h: Manages audio sources and configuration.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef SPECTRUM_CPP_AUDIO_MANAGER_H
#define SPECTRUM_CPP_AUDIO_MANAGER_H

#include "Common.h"

namespace Spectrum {

    class EventBus;
    class IAudioSource;

    class AudioManager {
    public:
        explicit AudioManager(EventBus* bus);
        ~AudioManager();

        bool Initialize();
        void Update(float deltaTime);
        SpectrumData GetSpectrum();

        void ToggleCapture();
        void ToggleAnimation();
        void ChangeAmplification(float delta);
        void ChangeBarCount(int delta);
        void ChangeFFTWindow(int direction);
        void ChangeSpectrumScale(int direction);

        bool IsCapturing() const { return m_isCapturing; }
        bool IsAnimating() const { return m_isAnimating; }

    private:
        std::unique_ptr<IAudioSource> m_realtimeSource;
        std::unique_ptr<IAudioSource> m_animatedSource;
        IAudioSource* m_currentSource = nullptr;

        AudioConfig m_audioConfig;
        bool m_isCapturing = false;
        bool m_isAnimating = false;
    };

}

#endif