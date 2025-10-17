// AudioCapture.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AudioCapture.cpp: Orchestrator for the audio capture process.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "AudioCapture.h"
#include "AudioCaptureEngine.h"
#include "WASAPIHelper.h"
#include <chrono>

namespace Spectrum {

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // PIMPL implementation structure.
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    struct AudioCapture::Implementation {
        std::unique_ptr<Internal::WasapiInitData> initData;
        std::unique_ptr<Internal::AudioPacketProcessor> processor;
        std::unique_ptr<Internal::ICaptureEngine> engine;

        std::thread captureThread;
        std::atomic<bool> isCapturing{ false };
        std::atomic<bool> stopRequested{ false };
        std::atomic<bool> isInitialized{ false };

        ~Implementation() {
            if (initData && initData->waveFormat) {
                CoTaskMemFree(initData->waveFormat);
                initData->waveFormat = nullptr;
            }
            if (initData && initData->samplesEvent) {
                CloseHandle(initData->samplesEvent);
                initData->samplesEvent = nullptr;
            }
        }

        void CaptureLoop() {
            WASAPI::ScopedCOMInitializer threadCom;
            if (engine && processor) {
                engine->Run(stopRequested, *processor);
            }
        }
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // AudioCapture public API implementation.
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    AudioCapture::AudioCapture() : m_pimpl(std::make_unique<Implementation>()) {}

    AudioCapture::~AudioCapture() {
        Stop();
    }

    bool AudioCapture::Initialize() {
        if (m_pimpl->isInitialized) {
            return true;
        }

        Internal::WasapiInitializer initializer;
        m_pimpl->initData = initializer.Initialize();

        if (!m_pimpl->initData) {
            LOG_ERROR("AudioCapture failed: WASAPI device initialization error.");
            return false;
        }

        auto* data = m_pimpl->initData.get();
        int channels = data->waveFormat ? data->waveFormat->nChannels : 0;

        m_pimpl->processor = std::make_unique<Internal::AudioPacketProcessor>(
            data->captureClient.Get(), channels
        );

        if (data->useEventMode) {
            m_pimpl->engine = std::make_unique<Internal::EventDrivenEngine>(
                data->samplesEvent
            );
        }
        else {
            m_pimpl->engine = std::make_unique<Internal::PollingEngine>();
            if (data->samplesEvent) {
                CloseHandle(data->samplesEvent);
                data->samplesEvent = nullptr;
            }
        }

        m_pimpl->isInitialized = true;
        LOG_INFO(
            "Audio capture initialized. Mode: "
            << (data->useEventMode ? "Event-driven" : "Polling")
        );
        LOG_INFO(
            "Format: " << GetSampleRate() << " Hz, " << GetChannels()
            << " channels, " << GetBitsPerSample() << " bits"
        );
        return true;
    }

    bool AudioCapture::Start() {
        if (!IsInitialized() || IsCapturing()) {
            return IsCapturing();
        }

        HRESULT hr = m_pimpl->initData->audioClient->Start();
        if (FAILED(hr)) {
            LOG_INFO("First audio client start attempt failed, retrying...");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            hr = m_pimpl->initData->audioClient->Start();
        }
        if (!WASAPI::CheckResult(hr, "Failed to start audio client")) {
            return false;
        }

        m_pimpl->stopRequested = false;
        m_pimpl->isCapturing = true;
        m_pimpl->captureThread = std::thread(
            &Implementation::CaptureLoop, m_pimpl.get()
        );
        LOG_INFO("Audio capture started.");
        return true;
    }

    void AudioCapture::Stop() noexcept {
        if (!IsCapturing()) {
            return;
        }

        m_pimpl->stopRequested = true;
        if (m_pimpl->initData && m_pimpl->initData->useEventMode
            && m_pimpl->initData->samplesEvent) {
            SetEvent(m_pimpl->initData->samplesEvent);
        }
        if (m_pimpl->captureThread.joinable()) {
            m_pimpl->captureThread.join();
        }
        if (m_pimpl->initData && m_pimpl->initData->audioClient) {
            m_pimpl->initData->audioClient->Stop();
        }

        m_pimpl->isCapturing = false;
        LOG_INFO("Audio capture stopped.");
    }

    void AudioCapture::SetCallback(IAudioCaptureCallback* callback) noexcept {
        if (m_pimpl->processor) {
            m_pimpl->processor->SetCallback(callback);
        }
    }

    bool AudioCapture::IsCapturing() const noexcept {
        return m_pimpl->isCapturing;
    }

    bool AudioCapture::IsInitialized() const noexcept {
        return m_pimpl->isInitialized;
    }

    int AudioCapture::GetSampleRate() const noexcept {
        return (m_pimpl->initData && m_pimpl->initData->waveFormat)
            ? static_cast<int>(m_pimpl->initData->waveFormat->nSamplesPerSec)
            : 0;
    }

    int AudioCapture::GetChannels() const noexcept {
        return (m_pimpl->initData && m_pimpl->initData->waveFormat)
            ? static_cast<int>(m_pimpl->initData->waveFormat->nChannels)
            : 0;
    }

    int AudioCapture::GetBitsPerSample() const noexcept {
        return (m_pimpl->initData && m_pimpl->initData->waveFormat)
            ? static_cast<int>(m_pimpl->initData->waveFormat->wBitsPerSample)
            : 0;
    }

} // namespace Spectrum