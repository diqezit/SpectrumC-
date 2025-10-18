#include "AudioManager.h"
#include "EventBus.h"
#include "Utils.h"
#include "RealtimeAudioSource.h"
#include "AnimatedAudioSource.h"

namespace Spectrum {

    AudioManager::AudioManager(EventBus* bus) {
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
        if (m_isCapturing && m_realtimeSource) {
            m_realtimeSource->StopCapture();
        }
    }

    bool AudioManager::Initialize() {
        m_realtimeSource = std::make_unique<RealtimeAudioSource>(m_audioConfig);
        m_animatedSource = std::make_unique<AnimatedAudioSource>(m_audioConfig);

        if (!m_realtimeSource->Initialize() || !m_animatedSource->Initialize()) {
            return false;
        }

        m_currentSource = m_realtimeSource.get();
        return true;
    }

    void AudioManager::Update(float deltaTime) {
        if (m_currentSource) {
            m_currentSource->Update(deltaTime);
        }
    }

    SpectrumData AudioManager::GetSpectrum() {
        if (m_currentSource) {
            return m_currentSource->GetSpectrum();
        }
        return {};
    }

    void AudioManager::ToggleCapture() {
        if (m_isAnimating) return;

        m_isCapturing = !m_isCapturing;
        if (m_isCapturing) {
            m_realtimeSource->StartCapture();
        }
        else {
            m_realtimeSource->StopCapture();
        }
    }

    void AudioManager::ToggleAnimation() {
        m_isAnimating = !m_isAnimating;
        if (m_isAnimating) {
            if (m_isCapturing) {
                m_realtimeSource->StopCapture();
                m_isCapturing = false;
            }
            m_currentSource = m_animatedSource.get();
            LOG_INFO("Animation mode ON.");
        }
        else {
            m_currentSource = m_realtimeSource.get();
            LOG_INFO("Animation mode OFF.");
        }
    }

    void AudioManager::ChangeAmplification(float delta) {
        m_audioConfig.amplification = Utils::Clamp(m_audioConfig.amplification + delta, 0.1f, 5.0f);
        if (m_realtimeSource) {
            m_realtimeSource->SetAmplification(m_audioConfig.amplification);
        }
        LOG_INFO("Amplification Factor: " << m_audioConfig.amplification);
    }

    void AudioManager::ChangeBarCount(int delta) {
        int newCount = static_cast<int>(m_audioConfig.barCount) + delta;
        m_audioConfig.barCount = Utils::Clamp<size_t>(newCount, 16, 256);
        if (m_realtimeSource) m_realtimeSource->SetBarCount(m_audioConfig.barCount);
        if (m_animatedSource) m_animatedSource->SetBarCount(m_audioConfig.barCount);
        LOG_INFO("Bar Count: " << m_audioConfig.barCount);
    }

    void AudioManager::ChangeFFTWindow(int direction) {
        m_audioConfig.windowType = Utils::CycleEnum(m_audioConfig.windowType, direction);
        if (m_realtimeSource) {
            m_realtimeSource->SetFFTWindow(m_audioConfig.windowType);
        }
        LOG_INFO("FFT Window: " << Utils::ToString(m_audioConfig.windowType));
    }

    void AudioManager::ChangeSpectrumScale(int direction) {
        m_audioConfig.scaleType = Utils::CycleEnum(m_audioConfig.scaleType, direction);
        if (m_realtimeSource) {
            m_realtimeSource->SetScaleType(m_audioConfig.scaleType);
        }
        LOG_INFO("Spectrum Scale: " << Utils::ToString(m_audioConfig.scaleType));
    }

}