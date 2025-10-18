#include "AnimatedAudioSource.h"
#include "Utils.h"

namespace Spectrum {

    AnimatedAudioSource::AnimatedAudioSource(const AudioConfig& config)
        : m_barCount(config.barCount), m_postProcessor(config.barCount)
    {
        m_postProcessor.SetSmoothing(config.smoothing);
    }

    void AnimatedAudioSource::Update(float deltaTime) {
        m_animationTime += deltaTime;
        SpectrumData testData = GenerateTestSpectrum(m_animationTime);
        m_postProcessor.Process(testData);
    }

    SpectrumData AnimatedAudioSource::GetSpectrum() {
        return m_postProcessor.GetSmoothedBars();
    }

    void AnimatedAudioSource::SetBarCount(size_t count) {
        if (m_barCount == count) return;
        m_barCount = count;
        m_postProcessor.SetBarCount(count);
    }

    void AnimatedAudioSource::SetSmoothing(float smoothing) {
        m_postProcessor.SetSmoothing(smoothing);
    }

    SpectrumData AnimatedAudioSource::GenerateTestSpectrum(float timeOffset) {
        SpectrumData testData(m_barCount, 0.0f);
        for (size_t i = 0; i < m_barCount; ++i) {
            const float frequency = static_cast<float>(i) / static_cast<float>(m_barCount);
            const float phase = timeOffset * 2.0f + static_cast<float>(i) * 0.3f;
            float value = (std::sin(phase) + 1.0f) * 0.5f;
            value *= (1.0f - frequency * 0.7f);
            value += Utils::Random::Instance().Float(-0.05f, 0.05f);
            testData[i] = Utils::Saturate(value);
        }
        return testData;
    }

}