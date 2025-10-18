// =-=-=-=-=-=-=-=-=-=-=
// ControllerCore.cpp
// =-=-=-=-=-=-=-=-=-=-=

#include "ControllerCore.h"
#include "WindowManager.h"
#include "AudioManager.h"
#include "RendererManager.h"
#include "InputManager.h"
#include "WindowHelper.h"
#include "GraphicsContext.h"
#include "UIManager.h"
#include "EventBus.h"

namespace Spectrum {

    ControllerCore::ControllerCore(HINSTANCE hInstance)
        : m_hInstance(hInstance) {
    }

    ControllerCore::~ControllerCore() = default;

    bool ControllerCore::Initialize() {
        if (!InitializeManagers()) {
            return false;
        }
        PrintWelcomeMessage();
        return true;
    }

    void ControllerCore::Run() {
        m_timer.Reset();
        MainLoop();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Initialization
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    bool ControllerCore::InitializeManagers() {
        m_eventBus = std::make_unique<EventBus>();

        m_windowManager = std::make_unique<WindowManager>(m_hInstance, this, m_eventBus.get());
        if (!m_windowManager->Initialize()) {
            return false;
        }

        m_inputManager = std::make_unique<InputManager>();

        m_audioManager = std::make_unique<AudioManager>(m_eventBus.get());
        if (!m_audioManager->Initialize()) {
            return false;
        }

        m_rendererManager = std::make_unique<RendererManager>(m_eventBus.get(), m_windowManager.get());
        if (!m_rendererManager->Initialize()) {
            return false;
        }

        // Set initial renderer
        m_rendererManager->SetCurrentRenderer(
            m_rendererManager->GetCurrentStyle(), m_windowManager->GetGraphics()
        );

        return true;
    }

    void ControllerCore::PrintWelcomeMessage() {
        LOG_INFO("========================================");
        LOG_INFO("     Spectrum Visualizer C++");
        LOG_INFO("========================================");
        LOG_INFO("Controls:");
        LOG_INFO("  SPACE - Toggle audio capture");
        LOG_INFO("  A     - Toggle animation (test mode)");
        LOG_INFO("  R     - Switch renderer");
        LOG_INFO("  Q     - Change render quality");
        LOG_INFO("  O     - Toggle Overlay Mode");
        LOG_INFO("  S     - Switch Spectrum Scale");
        LOG_INFO("  UP/DOWN Arrow  - Change Amplification");
        LOG_INFO("  LEFT/RIGHT Arrow - Change FFT Window");
        LOG_INFO("  -/+ Keys       - Change Bar Count");
        LOG_INFO("  ESC   - Exit");
        LOG_INFO("========================================");
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Main Application Loop
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    void ControllerCore::MainLoop() {
        while (m_windowManager->IsRunning()) {
            m_windowManager->ProcessMessages();

            // Run loop at a fixed framerate
            float dt = m_timer.GetElapsedSeconds();
            if (dt >= FRAME_TIME) {
                m_timer.Reset();
                ProcessInput();
                Update(dt);
                Render();
            }
            else {
                // Yield thread to avoid busy-waiting
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    void ControllerCore::ProcessInput() {
        m_inputManager->Update();
        m_actions = m_inputManager->GetActions();
        // Publish all queued actions to the event bus
        for (const auto& action : m_actions) {
            m_eventBus->Publish(action);
        }
    }

    void ControllerCore::Update(float deltaTime) {
        m_audioManager->Update(deltaTime);
    }

    void ControllerCore::Render() {
        auto* graphics = m_windowManager->GetGraphics();
        if (!graphics || !m_windowManager->IsActive()) {
            return;
        }

        // D2D can tell us if the window is occluded
        // No need to render if not visible
        if (auto* rt = graphics->GetRenderTarget()) {
            if (rt->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED) {
                return;
            }
        }

        graphics->BeginDraw();

        // Overlay mode requires a transparent background for composition
        const Color clearColor = m_windowManager->IsOverlayMode()
            ? Color::Transparent()
            : Color::FromRGB(13, 13, 26);
        graphics->Clear(clearColor);

        SpectrumData spectrum = m_audioManager->GetSpectrum();

        if (m_rendererManager->GetCurrentRenderer()) {
            m_rendererManager->GetCurrentRenderer()->Render(*graphics, spectrum);
        }

        // UI is not visible in overlay mode
        if (auto* uiManager = m_windowManager->GetUIManager()) {
            if (!m_windowManager->IsOverlayMode()) {
                uiManager->Draw(*graphics);
            }
        }

        HRESULT hr = graphics->EndDraw();
        // D2DERR_RECREATE_TARGET means the GPU device was lost
        // We must recreate all D2D resources
        if (hr == D2DERR_RECREATE_TARGET) {
            HWND hwnd = m_windowManager->GetCurrentHwnd();
            if (hwnd) {
                m_windowManager->RecreateGraphicsAndNotify(hwnd);
            }
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Callbacks & Event Handlers
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    void ControllerCore::OnResize(int width, int height) {
        if (m_windowManager) {
            auto* graphics = m_windowManager->GetGraphics();
            if (graphics) {
                graphics->Resize(width, height);
            }
        }
        if (m_rendererManager) {
            m_rendererManager->OnResize(width, height);
        }
    }

    void ControllerCore::SetPrimaryColor(const Color& color) {
        if (m_rendererManager && m_rendererManager->GetCurrentRenderer()) {
            m_rendererManager->GetCurrentRenderer()->SetPrimaryColor(color);
        }
    }

    void ControllerCore::OnClose() {
        if (m_windowManager && m_windowManager->IsRunning()) {
            if (auto* wnd = m_windowManager->GetMainWindow()) {
                // Triggers WM_CLOSE -> WM_DESTROY -> PostQuitMessage
                wnd->SetRunning(false);
            }
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Win32 Message Handling
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    LRESULT ControllerCore::HandleWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CLOSE:
            OnClose();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            if (wParam != SIZE_MINIMIZED) {
                OnResize(width, height);
            }
            return 0;
        }
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
            return HandleMouseMessage(msg, lParam);
        case WM_NCHITTEST:
            // Allows dragging the frameless overlay window
            if (m_windowManager->IsOverlayMode()) {
                return HTCAPTION;
            }
            break;
        case WM_ERASEBKGND:
            // Prevents flickering by telling Windows we handle all drawing
            return 1;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT ControllerCore::HandleMouseMessage(UINT msg, LPARAM lParam) {
        int x, y;
        WindowUtils::ExtractMouse(lParam, x, y);

        bool needsRedraw = false;
        if (auto* uiManager = m_windowManager->GetUIManager()) {
            needsRedraw = uiManager->HandleMouseMessage(msg, x, y);
        }

        // If UI interaction changed something visual, request a new frame
        if (needsRedraw) {
            HWND hwnd = m_windowManager->GetCurrentHwnd();
            if (hwnd) {
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        return 0;
    }
}