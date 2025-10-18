// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// IAudioSource.h: Interface for audio data providers.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef SPECTRUM_CPP_IAUDIOSOURCE_H
#define SPECTRUM_CPP_IAUDIOSOURCE_H

#include "Common.h"

namespace Spectrum {

    class IAudioSource {
    public:
        virtual ~IAudioSource() = default;

        virtual bool Initialize() = 0;
        virtual void Update(float deltaTime) = 0;
        virtual SpectrumData GetSpectrum() = 0;

        virtual void SetAmplification(float amp) {}
        virtual void SetBarCount(size_t count) {}
        virtual void SetFFTWindow(FFTWindowType type) {}
        virtual void SetScaleType(SpectrumScale type) {}

        virtual void StartCapture() {}
        virtual void StopCapture() {}
    };

}

#endif