#include "RealtimeAudioSource.h"

namespace Spectrum {

    RealtimeAudioSource::RealtimeAudioSource(const AudioConfig& config) {
        m_analyzer = std::make_unique<SpectrumAnalyzer>(config.barCount, config.fftSize);
        m_analyzer->SetAmplification(config.amplification);
        m_analyzer->SetSmoothing(config.smoothing);
        m_analyzer->SetFFTWindow(config.windowType);
        m_analyzer->SetScaleType(config.scaleType);
    }

    bool RealtimeAudioSource::Initialize() {
        m_audioCapture = std::make_unique<AudioCapture>();
        if (!m_audioCapture->Initialize()) {
            LOG_ERROR("Realtime source: Audio capture failed to initialize.");
        }
        if (m_audioCapture->IsInitialized()) {
            m_audioCapture->SetCallback(m_analyzer.get());
        }
        return true;
    }

    void RealtimeAudioSource::Update(float /*deltaTime*/) {
        m_analyzer->Update();
    }

    SpectrumData RealtimeAudioSource::GetSpectrum() {
        return m_analyzer->GetSpectrum();
    }

    void RealtimeAudioSource::StartCapture() {
        if (!m_isCapturing && m_audioCapture && m_audioCapture->Start()) {
            m_isCapturing = true;
            LOG_INFO("Realtime source: capture started.");
        }
    }

    void RealtimeAudioSource::StopCapture() {
        if (m_isCapturing && m_audioCapture) {
            m_audioCapture->Stop();
            m_isCapturing = false;
            LOG_INFO("Realtime source: capture stopped.");
        }
    }

    void RealtimeAudioSource::SetAmplification(float amp) { m_analyzer->SetAmplification(amp); }
    void RealtimeAudioSource::SetBarCount(size_t count) { m_analyzer->SetBarCount(count); }
    void RealtimeAudioSource::SetFFTWindow(FFTWindowType type) { m_analyzer->SetFFTWindow(type); }
    void RealtimeAudioSource::SetScaleType(SpectrumScale type) { m_analyzer->SetScaleType(type); }

}