// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AudioCapture.cpp: Implementation for a single audio capture session.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "AudioCapture.h"
#include "AudioCaptureEngine.h"
#include "WASAPIHelper.h"
#include <chrono>

namespace Spectrum {

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // PIMPL implementation
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    struct AudioCapture::Implementation {
        std::unique_ptr<Internal::WasapiInitData> initData;
        std::unique_ptr<Internal::AudioPacketProcessor> processor;
        std::unique_ptr<Internal::ICaptureEngine> engine;

        std::thread captureThread;
        std::atomic<bool> isCapturing{ false };
        std::atomic<bool> stopRequested{ false };
        std::atomic<bool> isInitialized{ false };
        std::atomic<bool> isFaulted{ false };
        HRESULT lastError{ S_OK };

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
            if (!threadCom.IsInitialized()) {
                isFaulted = true;
                lastError = CO_E_NOTINITIALIZED;
                return;
            }

            if (engine && processor) {
                lastError = engine->Run(stopRequested, *processor);
                if (FAILED(lastError) && !stopRequested) {
                    isFaulted = true;
                    if (lastError == AUDCLNT_E_DEVICE_INVALIDATED) {
                        LOG_ERROR("Audio device was lost.");
                    }
                    else {
                        LOG_ERROR("Audio capture thread exited with error: " << lastError);
                    }
                }
            }
        }
    };

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Public API implementation
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    AudioCapture::AudioCapture() : m_pimpl(std::make_unique<Implementation>()) {}
    AudioCapture::~AudioCapture() { Stop(); }

    bool AudioCapture::Initialize() {
        if (m_pimpl->isInitialized) {
            return true;
        }

        m_pimpl->isFaulted = false;
        m_pimpl->lastError = S_OK;

        Internal::WasapiInitializer initializer;
        m_pimpl->initData = initializer.Initialize();

        if (!m_pimpl->initData) {
            m_pimpl->isFaulted = true;
            return false;
        }

        auto* data = m_pimpl->initData.get();
        int channels = data->waveFormat ? data->waveFormat->nChannels : 0;

        m_pimpl->processor = std::make_unique<Internal::AudioPacketProcessor>(
            data->captureClient.Get(),
            channels
        );

        if (data->useEventMode) {
            m_pimpl->engine = std::make_unique<Internal::EventDrivenEngine>(data->samplesEvent);
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
        if (!IsInitialized() || IsCapturing() || IsFaulted()) {
            return false;
        }

        HRESULT hr = m_pimpl->initData->audioClient->Start();
        if (!WASAPI::CheckResult(hr, "Failed to start audio client")) {
            m_pimpl->isFaulted = true;
            m_pimpl->lastError = hr;
            return false;
        }

        m_pimpl->stopRequested = false;
        m_pimpl->isCapturing = true;
        m_pimpl->captureThread = std::thread(&Implementation::CaptureLoop, m_pimpl.get());
        return true;
    }

    void AudioCapture::Stop() noexcept {
        if (!m_pimpl->isCapturing && !m_pimpl->captureThread.joinable()) {
            return;
        }

        m_pimpl->stopRequested = true;
        if (m_pimpl->initData && m_pimpl->initData->useEventMode && m_pimpl->initData->samplesEvent) {
            SetEvent(m_pimpl->initData->samplesEvent);
        }
        if (m_pimpl->captureThread.joinable()) {
            m_pimpl->captureThread.join();
        }
        if (m_pimpl->initData && m_pimpl->initData->audioClient) {
            m_pimpl->initData->audioClient->Stop();
        }

        m_pimpl->isCapturing = false;
    }

    void AudioCapture::SetCallback(IAudioCaptureCallback* callback) noexcept {
        if (m_pimpl->processor) {
            m_pimpl->processor->SetCallback(callback);
        }
    }

    bool AudioCapture::IsCapturing() const noexcept { return m_pimpl->isCapturing; }
    bool AudioCapture::IsInitialized() const noexcept { return m_pimpl->isInitialized; }
    bool AudioCapture::IsFaulted() const noexcept { return m_pimpl->isFaulted; }
    HRESULT AudioCapture::GetLastError() const noexcept { return m_pimpl->lastError; }

    int AudioCapture::GetSampleRate() const noexcept {
        if (m_pimpl->initData && m_pimpl->initData->waveFormat) {
            return static_cast<int>(m_pimpl->initData->waveFormat->nSamplesPerSec);
        }
        return 0;
    }

    int AudioCapture::GetChannels() const noexcept {
        if (m_pimpl->initData && m_pimpl->initData->waveFormat) {
            return static_cast<int>(m_pimpl->initData->waveFormat->nChannels);
        }
        return 0;
    }

    int AudioCapture::GetBitsPerSample() const noexcept {
        if (m_pimpl->initData && m_pimpl->initData->waveFormat) {
            return static_cast<int>(m_pimpl->initData->waveFormat->wBitsPerSample);
        }
        return 0;
    }

}