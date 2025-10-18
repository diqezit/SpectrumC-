// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AnimatedAudioSource.h: Provides procedurally generated spectrum data.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef SPECTRUM_CPP_ANIMATEDAUDIOSOURCE_H
#define SPECTRUM_CPP_ANIMATEDAUDIOSOURCE_H

#include "IAudioSource.h"
#include "SpectrumPostProcessor.h"

namespace Spectrum {

    class AnimatedAudioSource : public IAudioSource {
    public:
        explicit AnimatedAudioSource(const AudioConfig& config);

        bool Initialize() override { return true; }
        void Update(float deltaTime) override;
        SpectrumData GetSpectrum() override;

        void SetBarCount(size_t count) override;
        void SetSmoothing(float smoothing);

    private:
        SpectrumData GenerateTestSpectrum(float timeOffset);

        float m_animationTime = 0.0f;
        size_t m_barCount;
        SpectrumPostProcessor m_postProcessor;
    };

}

#endif