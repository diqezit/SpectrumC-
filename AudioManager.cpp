// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AudioManager.cpp: Implementation of AudioManager.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "AudioManager.h"
#include "Utils.h"
#include "EventBus.h"

namespace Spectrum {

    AudioManager::AudioManager(EventBus* bus)
        : m_isCapturing(false), m_isAnimating(false), m_animationTime(0.0f)
    {
        bus->Subscribe(InputAction::ToggleCapture, [this]() { this->ToggleCapture(); });
        bus->Subscribe(InputAction::ToggleAnimation, [this]() { this->ToggleAnimation(); });
        bus->Subscribe(InputAction::CycleSpectrumScale, [this]() { this->ChangeSpectrumScale(1); });
        bus->Subscribe(InputAction::IncreaseAmplification, [this]() { this->ChangeAmplification(0.1f); });
        bus->Subscribe(InputAction::DecreaseAmplification, [this]() { this->ChangeAmplification(-0.1f); });
        bus->Subscribe(InputAction::NextFFTWindow, [this]() { this->ChangeFFTWindow(1); });
        bus->Subscribe(InputAction::PrevFFTWindow, [this]() { this->ChangeFFTWindow(-1); });
        bus->Subscribe(InputAction::IncreaseBarCount, [this]() { this->ChangeBarCount(4); });
        bus->Subscribe(InputAction::DecreaseBarCount, [this]() { this->ChangeBarCount(-4); });
    }

    AudioManager::~AudioManager() {
        if (m_audioCapture && m_isCapturing) {
            m_audioCapture->Stop();
        }
    }

    bool AudioManager::Initialize() {
        if (!InitializeAudioCapture()) {
            return false;
        }
        if (!InitializeAnalyzer()) {
            return false;
        }

        if (m_audioCapture->IsInitialized()) {
            m_audioCapture->SetCallback(m_analyzer.get());
        }

        return true;
    }

    bool AudioManager::InitializeAudioCapture() {
        m_audioCapture = std::make_unique<AudioCapture>();
        if (!m_audioCapture->Initialize()) {
            LOG_ERROR("Audio capture failed to initialize. Test mode only.");
        }
        return true;
    }

    bool AudioManager::InitializeAnalyzer() {
        m_analyzer = std::make_unique<SpectrumAnalyzer>(
            m_audioConfig.barCount, m_audioConfig.fftSize
        );

        m_analyzer->SetAmplification(m_audioConfig.amplification);
        m_analyzer->SetSmoothing(m_audioConfig.smoothing);
        m_analyzer->SetFFTWindow(m_audioConfig.windowType);
        m_analyzer->SetScaleType(m_audioConfig.scaleType);
        return true;
    }

    void AudioManager::Update(float deltaTime) {
        if (m_isAnimating) {
            m_animationTime += deltaTime;
            m_analyzer->GenerateTestData(m_animationTime);
        }
        else {
            m_analyzer->Update();
        }
    }

    SpectrumData AudioManager::GetSpectrum() {
        return m_analyzer->GetSpectrum();
    }

    void AudioManager::ToggleCapture() {
        if (m_isCapturing) {
            StopCaptureInternal();
        }
        else {
            StartCaptureInternal();
        }
    }

    bool AudioManager::StartCaptureInternal() {
        if (!m_audioCapture || !m_audioCapture->IsInitialized()) {
            LOG_ERROR("Audio capture not available.");
            return false;
        }
        if (!m_audioCapture->Start()) {
            LOG_ERROR("Failed to start audio capture.");
            return false;
        }
        m_isCapturing = true;
        m_isAnimating = false;
        LOG_INFO("Audio capture started.");
        return true;
    }

    void AudioManager::StopCaptureInternal() {
        if (m_audioCapture) {
            m_audioCapture->Stop();
        }
        m_isCapturing = false;
        LOG_INFO("Audio capture stopped.");
    }

    void AudioManager::ToggleAnimation() {
        m_isAnimating = !m_isAnimating;

        if (m_isAnimating) {
            if (m_isCapturing) {
                StopCaptureInternal();
            }
            LOG_INFO("Animation mode ON.");
        }
        else {
            LOG_INFO("Animation mode OFF.");
        }
    }

    void AudioManager::ChangeAmplification(float delta) {
        m_audioConfig.amplification += delta;
        m_analyzer->SetAmplification(m_audioConfig.amplification);
        LOG_INFO(
            "Amplification Factor: " << GetCurrentAmplification()
        );
    }

    void AudioManager::ChangeBarCount(int delta) {
        int newCount = static_cast<int>(m_audioConfig.barCount) + delta;
        m_audioConfig.barCount = Utils::Clamp<size_t>(
            newCount, 16, 256
        );
        m_analyzer->SetBarCount(m_audioConfig.barCount);
        LOG_INFO("Bar Count: " << m_audioConfig.barCount);
    }

    void AudioManager::ChangeFFTWindow(int direction) {
        m_audioConfig.windowType = Utils::CycleEnum(
            m_audioConfig.windowType, direction
        );
        m_analyzer->SetFFTWindow(m_audioConfig.windowType);
        LOG_INFO(
            "FFT Window: " << Utils::ToString(m_audioConfig.windowType)
        );
    }

    void AudioManager::ChangeSpectrumScale(int direction) {
        m_audioConfig.scaleType = Utils::CycleEnum(
            m_audioConfig.scaleType, direction
        );
        m_analyzer->SetScaleType(m_audioConfig.scaleType);
        LOG_INFO(
            "Spectrum Scale: " << Utils::ToString(m_audioConfig.scaleType)
        );
    }

}