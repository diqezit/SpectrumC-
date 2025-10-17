// AudioCaptureEngine.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// AudioCaptureEngine.cpp: Implementation of the internal audio capture logic.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "AudioCaptureEngine.h"
#include "AudioCapture.h"
#include "WASAPIHelper.h"
#include <chrono>

namespace Spectrum {
    namespace Internal {

        using namespace WASAPI;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // WasapiInitializer Implementation
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        std::unique_ptr<WasapiInitData> WasapiInitializer::Initialize() {
            static constexpr int MAX_INIT_RETRIES = 3;
            static constexpr DWORD INIT_RETRY_DELAY_MS = 200;

            ScopedCOMInitializer com;
            if (!com.IsInitialized()) {
                return nullptr;
            }

            wrl::ComPtr<IMMDevice> device;
            for (int retry = 0; retry < MAX_INIT_RETRIES; ++retry) {
                if (!InitializeDevice(device)) {
                    continue;
                }

                auto data = std::make_unique<WasapiInitData>();
                if (InitializeClient(device, *data)) {
                    return data;
                }

                if (retry < MAX_INIT_RETRIES - 1) {
                    LOG_INFO(
                        "Initialization attempt " << (retry + 1)
                        << " failed, retrying..."
                    );
                    std::this_thread::sleep_for(
                        std::chrono::milliseconds(INIT_RETRY_DELAY_MS)
                    );
                }
            }
            LOG_ERROR(
                "Failed to initialize audio capture after " << MAX_INIT_RETRIES
                << " attempts"
            );
            return nullptr;
        }

        bool WasapiInitializer::InitializeDevice(
            wrl::ComPtr<IMMDevice>& device
        ) const {
            wrl::ComPtr<IMMDeviceEnumerator> enumerator;
            HRESULT hr = CoCreateInstance(
                __uuidof(MMDeviceEnumerator),
                nullptr,
                CLSCTX_ALL,
                __uuidof(IMMDeviceEnumerator),
                &enumerator
            );
            if (!CheckResult(hr, "Failed to create device enumerator")) {
                return false;
            }

            hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
            return CheckResult(hr, "Failed to get default audio endpoint");
        }

        bool WasapiInitializer::InitializeClient(
            wrl::ComPtr<IMMDevice>& device, WasapiInitData& data
        ) const {
            HRESULT hr = device->Activate(
                __uuidof(IAudioClient), CLSCTX_ALL, nullptr, &data.audioClient
            );
            if (!CheckResult(hr, "Failed to activate audio client")) {
                return false;
            }

            hr = data.audioClient->GetMixFormat(&data.waveFormat);
            if (!CheckResult(hr, "Failed to get mix format")) {
                return false;
            }

            data.samplesEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (!data.samplesEvent) {
                LOG_ERROR("Failed to create capture event");
                return false;
            }

            const DWORD eventFlags = AUDCLNT_STREAMFLAGS_LOOPBACK |
                AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
                AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM;

            if (TryInitializeMode(data.audioClient.Get(), data.waveFormat,
                eventFlags, true, data.samplesEvent)) {
                data.useEventMode = true;
            }
            else {
                ResetClient(device, data.audioClient);
                const DWORD pollingFlags = AUDCLNT_STREAMFLAGS_LOOPBACK |
                    AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM;
                if (TryInitializeMode(data.audioClient.Get(), data.waveFormat,
                    pollingFlags, false, nullptr)) {
                    data.useEventMode = false;
                }
                else {
                    return false;
                }
            }

            return SetupCaptureClient(
                data.audioClient.Get(), data.captureClient.GetAddressOf()
            );
        }

        bool WasapiInitializer::TryInitializeMode(
            IAudioClient* client, WAVEFORMATEX* wf,
            DWORD flags, bool setEvent, HANDLE h
        ) const {
            static constexpr REFERENCE_TIME REFTIMES_PER_SEC = 10000000;
            static constexpr REFERENCE_TIME BUFFER_DURATION = REFTIMES_PER_SEC / 2;

            HRESULT hr = client->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                flags,
                BUFFER_DURATION,
                0, // Periodicity
                wf,
                nullptr // AudioSessionGuid
            );
            if (SUCCEEDED(hr) && setEvent) {
                hr = client->SetEventHandle(h);
            }
            return SUCCEEDED(hr);
        }

        void WasapiInitializer::ResetClient(
            wrl::ComPtr<IMMDevice>& device, wrl::ComPtr<IAudioClient>& client
        ) const {
            client.Reset();
            if (device) {
                device->Activate(
                    __uuidof(IAudioClient), CLSCTX_ALL, nullptr, &client
                );
            }
        }

        bool WasapiInitializer::SetupCaptureClient(
            IAudioClient* audioClient, IAudioCaptureClient** captureClient
        ) const {
            HRESULT hr = audioClient->GetService(
                __uuidof(IAudioCaptureClient),
                reinterpret_cast<void**>(captureClient)
            );
            return CheckResult(hr, "Failed to get capture client service");
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // AudioPacketProcessor Implementation
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        AudioPacketProcessor::AudioPacketProcessor(
            IAudioCaptureClient* client, int channels
        ) : m_captureClient(client), m_channels(channels), m_callback(nullptr) {
        }

        void AudioPacketProcessor::SetCallback(
            IAudioCaptureCallback* callback
        ) noexcept {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            m_callback = callback;
        }

        bool AudioPacketProcessor::ProcessAvailablePackets() {
            UINT32 packetLen = 0;
            while (SUCCEEDED(m_captureClient->GetNextPacketSize(&packetLen))
                && packetLen > 0) {
                BYTE* data = nullptr;
                UINT32 frames = 0;
                DWORD flags = 0;

                HRESULT hr = m_captureClient->GetBuffer(
                    &data, &frames, &flags, nullptr, nullptr
                );
                if (FAILED(hr)) {
                    return false;
                }

                if (frames > 0 && data != nullptr
                    && !(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                    std::lock_guard<std::mutex> lock(m_callbackMutex);
                    if (m_callback) {
                        m_callback->OnAudioData(
                            reinterpret_cast<float*>(data),
                            static_cast<size_t>(frames) * m_channels,
                            m_channels
                        );
                    }
                }

                hr = m_captureClient->ReleaseBuffer(frames);
                if (FAILED(hr)) {
                    return false;
                }
            }
            return true;
        }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Capture Engine Implementations
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        EventDrivenEngine::EventDrivenEngine(HANDLE event) : m_samplesEvent(event) {}

        void EventDrivenEngine::Run(
            const std::atomic<bool>& stopRequested,
            AudioPacketProcessor& processor
        ) {
            constexpr DWORD kWaitTimeoutMs = 2000;
            while (!stopRequested) {
                DWORD waitResult = WaitForSingleObject(
                    m_samplesEvent, kWaitTimeoutMs
                );
                if (waitResult == WAIT_OBJECT_0) {
                    if (!processor.ProcessAvailablePackets()) {
                        break;
                    }
                }
                else if (waitResult != WAIT_TIMEOUT) {
                    if (!stopRequested) {
                        LOG_ERROR("Event-driven capture loop failed on wait.");
                    }
                    break;
                }
            }
        }

        void PollingEngine::Run(
            const std::atomic<bool>& stopRequested,
            AudioPacketProcessor& processor
        ) {
            constexpr DWORD POLLING_INTERVAL_MS = 10;
            while (!stopRequested) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(POLLING_INTERVAL_MS)
                );
                if (!processor.ProcessAvailablePackets()) {
                    if (!stopRequested) {
                        LOG_ERROR("Error processing packets in polling mode.");
                    }
                    break;
                }
            }
        }

    } // namespace Internal
} // namespace Spectrum