#ifndef SPECTRUM_CPP_SPECTRUM_POST_PROCESSOR_H
#define SPECTRUM_CPP_SPECTRUM_POST_PROCESSOR_H

#include "Common.h"

namespace Spectrum {

    class SpectrumPostProcessor {
    public:
        explicit SpectrumPostProcessor(size_t barCount);

        void Process(SpectrumData& spectrum);
        void Reset();

        void SetBarCount(size_t newBarCount);
        void SetAmplification(float newAmplification);
        void SetSmoothing(float newSmoothing);

        const SpectrumData& GetSmoothedBars() const { return m_smoothedBars; }
        const SpectrumData& GetPeakValues() const { return m_peakValues; }
        float GetAmplification() const { return m_amplificationFactor; }
        float GetSmoothing() const { return m_smoothingFactor; }

    private:
        void ApplyScaling(SpectrumData& spectrum);
        void UpdatePeakValues(const SpectrumData& spectrum);
        void ApplySmoothing(SpectrumData& spectrum);

        size_t m_barCount;
        float m_amplificationFactor;
        float m_smoothingFactor;

        SpectrumData m_smoothedBars;
        SpectrumData m_peakValues;
    };

}

#endif