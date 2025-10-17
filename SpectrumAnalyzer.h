#ifndef SPECTRUM_CPP_SPECTRUM_ANALYZER_H
#define SPECTRUM_CPP_SPECTRUM_ANALYZER_H

#include "Common.h"
#include "FFTProcessor.h"
#include "FrequencyMapper.h"
#include "SpectrumPostProcessor.h"
#include "AudioCapture.h"

namespace Spectrum {

    class SpectrumAnalyzer : public IAudioCaptureCallback {
    private:
        class AudioBufferManager {
        public:
            void Add(const float* data, size_t frames, int channels);
            bool HasEnoughData(size_t required) const;
            void CopyTo(AudioBuffer& dest, size_t size);
            void Consume(size_t size);

        private:
            AudioBuffer m_buffer;
            std::mutex m_mutex;
        };

    public:
        SpectrumAnalyzer(
            size_t barCount = DEFAULT_BAR_COUNT,
            size_t fftSize = DEFAULT_FFT_SIZE
        );

        void OnAudioData(const float* data, size_t samples, int channels) override;
        void Update();
        void GenerateTestData(float timeOffset);

        void SetBarCount(size_t newBarCount);
        void SetAmplification(float newAmplification);
        void SetSmoothing(float newSmoothing);
        void SetFFTWindow(FFTWindowType windowType);
        void SetScaleType(SpectrumScale scaleType);

        SpectrumData GetSpectrum();
        const SpectrumData& GetPeakValues() const;
        size_t GetBarCount() const;
        float GetAmplification() const;
        float GetSmoothing() const;
        SpectrumScale GetScaleType() const;

    private:
        void ProcessSingleFFTChunk();
        SpectrumData GenerateTestSpectrum(float timeOffset);

        size_t m_barCount;
        SpectrumScale m_scaleType;
        size_t m_sampleRate;

        FFTProcessor m_fftProcessor;
        FrequencyMapper m_frequencyMapper;
        SpectrumPostProcessor m_postProcessor;
        AudioBufferManager m_bufferManager;

        AudioBuffer m_processBuffer;
        std::mutex m_mutex;
    };

}

#endif