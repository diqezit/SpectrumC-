// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AudioManager.h: Manages audio capture and analysis.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef SPECTRUM_CPP_AUDIO_MANAGER_H
#define SPECTRUM_CPP_AUDIO_MANAGER_H

#include "Common.h"
#include "AudioCapture.h"
#include "SpectrumAnalyzer.h"

namespace Spectrum {

    class EventBus;

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

        FFTWindowType GetCurrentFFTWindowType() const {
            return m_audioConfig.windowType;
        }
        SpectrumScale GetCurrentSpectrumScale() const {
            return m_audioConfig.scaleType;
        }
        float GetCurrentAmplification() const {
            return m_analyzer ? m_analyzer->GetAmplification() : 0.0f;
        }
        size_t GetCurrentBarCount() const {
            return m_audioConfig.barCount;
        }

    private:
        bool InitializeAudioCapture();
        bool InitializeAnalyzer();
        bool StartCaptureInternal();
        void StopCaptureInternal();

        std::unique_ptr<AudioCapture> m_audioCapture;
        std::unique_ptr<SpectrumAnalyzer> m_analyzer;

        AudioConfig m_audioConfig;
        bool m_isCapturing;
        bool m_isAnimating;
        float m_animationTime;
    };

}

#endif