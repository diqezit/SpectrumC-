#include "SpectrumAnalyzer.h"
#include "Utils.h"

namespace Spectrum {

    void SpectrumAnalyzer::AudioBufferManager::Add(
        const float* data,
        size_t frames,
        int channels
    ) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_buffer.reserve(m_buffer.size() + frames);
        const float invChannels = 1.0f / static_cast<float>(channels);

        for (size_t frame = 0; frame < frames; ++frame) {
            const float* frameData = data + frame * static_cast<size_t>(channels);
            float monoSample = 0.0f;
            for (int ch = 0; ch < channels; ++ch) {
                monoSample += frameData[ch];
            }
            m_buffer.push_back(monoSample * invChannels);
        }
    }

    bool SpectrumAnalyzer::AudioBufferManager::HasEnoughData(
        size_t required
    ) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
        return m_buffer.size() >= required;
    }

    void SpectrumAnalyzer::AudioBufferManager::CopyTo(
        AudioBuffer& dest,
        size_t size
    ) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::copy_n(m_buffer.begin(), size, dest.begin());
    }

    void SpectrumAnalyzer::AudioBufferManager::Consume(size_t size) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + size);
    }

    SpectrumAnalyzer::SpectrumAnalyzer(size_t barCount, size_t fftSize)
        : m_barCount(barCount),
        m_scaleType(SpectrumScale::Logarithmic),
        m_sampleRate(DEFAULT_SAMPLE_RATE),
        m_fftProcessor(fftSize),
        m_frequencyMapper(barCount, DEFAULT_SAMPLE_RATE),
        m_postProcessor(barCount) {
        m_processBuffer.resize(fftSize);
    }

    void SpectrumAnalyzer::OnAudioData(
        const float* data,
        size_t samples,
        int channels
    ) {
        if (!data || samples == 0 || channels <= 0) return;
        const size_t frames = samples / static_cast<size_t>(channels);
        if (frames == 0) return;

        m_bufferManager.Add(data, frames, channels);
    }

    void SpectrumAnalyzer::Update() {
        const size_t fftSize = m_fftProcessor.GetFFTSize();
        const size_t hopSize = fftSize / 2;

        while (m_bufferManager.HasEnoughData(fftSize)) {
            ProcessSingleFFTChunk();
            m_bufferManager.Consume(hopSize);
        }
    }

    void SpectrumAnalyzer::ProcessSingleFFTChunk() {
        m_bufferManager.CopyTo(m_processBuffer, m_fftProcessor.GetFFTSize());
        m_fftProcessor.Process(m_processBuffer);

        SpectrumData currentBars(m_barCount, 0.0f);
        m_frequencyMapper.MapFFTToBars(
            m_fftProcessor.GetMagnitudes(),
            currentBars,
            m_scaleType
        );

        std::lock_guard<std::mutex> lock(m_mutex);
        m_postProcessor.Process(currentBars);
    }

    SpectrumData SpectrumAnalyzer::GetSpectrum() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_postProcessor.GetSmoothedBars();
    }

    SpectrumData SpectrumAnalyzer::GenerateTestSpectrum(float timeOffset) {
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

    void SpectrumAnalyzer::GenerateTestData(float timeOffset) {
        SpectrumData testData = GenerateTestSpectrum(timeOffset);
        std::lock_guard<std::mutex> lock(m_mutex);
        m_postProcessor.Process(testData);
    }

    void SpectrumAnalyzer::SetBarCount(size_t newBarCount) {
        if (newBarCount == 0 || newBarCount == m_barCount) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_barCount = newBarCount;
        m_frequencyMapper.SetBarCount(newBarCount);
        m_postProcessor.SetBarCount(newBarCount);
    }

    void SpectrumAnalyzer::SetAmplification(float newAmplification) {
        m_postProcessor.SetAmplification(newAmplification);
    }

    void SpectrumAnalyzer::SetSmoothing(float newSmoothing) {
        m_postProcessor.SetSmoothing(newSmoothing);
    }

    void SpectrumAnalyzer::SetFFTWindow(FFTWindowType windowType) {
        m_fftProcessor.SetWindowType(windowType);
    }

    void SpectrumAnalyzer::SetScaleType(SpectrumScale scaleType) {
        m_scaleType = scaleType;
    }

    const SpectrumData& SpectrumAnalyzer::GetPeakValues() const {
        return m_postProcessor.GetPeakValues();
    }
    size_t SpectrumAnalyzer::GetBarCount() const { return m_barCount; }
    float SpectrumAnalyzer::GetAmplification() const {
        return m_postProcessor.GetAmplification();
    }
    float SpectrumAnalyzer::GetSmoothing() const {
        return m_postProcessor.GetSmoothing();
    }
    SpectrumScale SpectrumAnalyzer::GetScaleType() const { return m_scaleType; }

}