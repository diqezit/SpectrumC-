#include "SpectrumPostProcessor.h"
#include "Utils.h"

namespace Spectrum {

    SpectrumPostProcessor::SpectrumPostProcessor(size_t barCount)
        : m_barCount(barCount),
        m_amplificationFactor(DEFAULT_AMPLIFICATION),
        m_smoothingFactor(DEFAULT_SMOOTHING) {
        Reset();
    }

    void SpectrumPostProcessor::Process(SpectrumData& spectrum) {
        if (spectrum.size() != m_barCount) return;
        ApplyScaling(spectrum);
        UpdatePeakValues(spectrum);
        ApplySmoothing(spectrum);
    }

    void SpectrumPostProcessor::Reset() {
        m_smoothedBars.assign(m_barCount, 0.0f);
        m_peakValues.assign(m_barCount, 0.0f);
    }

    void SpectrumPostProcessor::SetBarCount(size_t newBarCount) {
        if (newBarCount > 0 && newBarCount != m_barCount) {
            m_barCount = newBarCount;
            Reset();
        }
    }

    void SpectrumPostProcessor::SetAmplification(float newAmplification) {
        m_amplificationFactor = Utils::Clamp(newAmplification, 0.1f, 5.0f);
    }

    void SpectrumPostProcessor::SetSmoothing(float newSmoothing) {
        m_smoothingFactor = Utils::Saturate(newSmoothing);
    }

    void SpectrumPostProcessor::ApplyScaling(SpectrumData& spectrum) {
        const float sensitivity = 150.0f;
        for (size_t i = 0; i < m_barCount; ++i) {
            float scaled = std::log1p(spectrum[i] * sensitivity) / std::log1p(sensitivity);
            scaled = std::pow(scaled, m_amplificationFactor);
            spectrum[i] = Utils::Saturate(scaled);
        }
    }

    void SpectrumPostProcessor::UpdatePeakValues(const SpectrumData& spectrum) {
        const float peakDecayRate = 0.98f;
        for (size_t i = 0; i < m_barCount; ++i) {
            if (spectrum[i] > m_peakValues[i]) {
                m_peakValues[i] = spectrum[i];
            }
            else {
                m_peakValues[i] *= peakDecayRate;
            }
        }
    }

    void SpectrumPostProcessor::ApplySmoothing(SpectrumData& spectrum) {
        const float attackSmoothingFactor = 0.5f;
        for (size_t i = 0; i < m_barCount; ++i) {
            const float smoothing = (spectrum[i] > m_smoothedBars[i])
                ? m_smoothingFactor * attackSmoothingFactor
                : m_smoothingFactor;

            m_smoothedBars[i] = m_smoothedBars[i] * smoothing + spectrum[i] * (1.0f - smoothing);
        }
    }

}