// =-=-=-=-=-=-=-=-=-=-=
// WindowManager.cpp
// =-=-=-=-=-=-=-=-=-=-=

#include "WindowManager.h"
#include "ControllerCore.h"
#include "WindowHelper.h"
#include "EventBus.h"
#include "GraphicsContext.h"
#include "UIManager.h"
#include "RendererManager.h"

namespace Spectrum {

    WindowManager::WindowManager(
        HINSTANCE hInstance,
        ControllerCore* controller,
        EventBus* bus
    ) : m_hInstance(hInstance),
        m_controller(controller),
        m_isOverlay(false)
    {
        m_uiManager = std::make_unique<UIManager>(m_controller);

        bus->Subscribe(InputAction::ToggleOverlay, [this]() {
            this->ToggleOverlay();
            });

        bus->Subscribe(InputAction::Exit, [this]() {
            // In overlay mode ESC reverts to main window
            if (this->IsOverlayMode()) {
                this->ToggleOverlay();
            }
            // In main window it closes application
            else if (m_controller) {
                m_controller->OnClose();
            }
            });
    }

    WindowManager::~WindowManager() = default;

    bool WindowManager::Initialize() {
        if (!InitializeMainWindow()) return false;
        if (!InitializeOverlayWindow()) return false;
        if (!RecreateGraphicsAndNotify(m_mainWnd->GetHwnd())) return false;
        if (m_uiManager && !m_uiManager->Initialize(*m_graphics)) return false;

        WindowUtils::CenterOnScreen(m_mainWnd->GetHwnd());
        m_mainWnd->Show();
        return true;
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Initialization
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    bool WindowManager::InitializeMainWindow() {
        m_mainWnd = std::make_unique<MainWindow>(m_hInstance);
        return m_mainWnd->Initialize(
            L"Spectrum Visualizer", 800, 600, false, this
        );
    }

    bool WindowManager::InitializeOverlayWindow() {
        int screenW, screenH;
        WindowUtils::GetScreenSize(screenW, screenH);
        m_overlayWnd = std::make_unique<MainWindow>(m_hInstance);
        return m_overlayWnd->Initialize(
            L"Spectrum Overlay", screenW, 300, true, this
        );
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Main Loop & State
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    void WindowManager::ProcessMessages() {
        if (m_mainWnd && m_mainWnd->IsRunning()) {
            m_mainWnd->ProcessMessages();
        }
    }

    bool WindowManager::IsRunning() const {
        return m_mainWnd && m_mainWnd->IsRunning();
    }

    // Checks if the active window is visible and not minimized
    bool WindowManager::IsActive() const {
        if (!IsRunning()) return false;
        HWND hwnd = GetCurrentHwnd();
        return IsWindow(hwnd) && IsWindowVisible(hwnd) && !IsIconic(hwnd);
    }

    HWND WindowManager::GetCurrentHwnd() const {
        return m_isOverlay
            ? (m_overlayWnd ? m_overlayWnd->GetHwnd() : nullptr)
            : (m_mainWnd ? m_mainWnd->GetHwnd() : nullptr);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Graphics & Overlay Management
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    bool WindowManager::RecreateGraphicsAndNotify(HWND hwnd) {
        if (!hwnd) return false;

        m_graphics.reset();
        m_graphics = std::make_unique<GraphicsContext>(hwnd);
        if (!m_graphics->Initialize()) return false;

        if (m_uiManager) {
            m_uiManager->RecreateResources(*m_graphics);
        }

        if (m_controller) {
            RECT rc;
            GetClientRect(hwnd, &rc);
            m_controller->OnResize(rc.right - rc.left, rc.bottom - rc.top);
        }
        return true;
    }

    // Hides one window and shows the other
    // Also notifies the renderer to adjust its settings
    void WindowManager::ToggleOverlay() {
        m_isOverlay = !m_isOverlay;

        if (m_isOverlay) {
            ActivateOverlayMode();
        }
        else {
            DeactivateOverlayMode();
        }

        // Notify current renderer about the mode change
        if (m_controller) {
            if (auto* rendererManager = m_controller->GetRendererManager()) {
                if (auto* renderer = rendererManager->GetCurrentRenderer()) {
                    renderer->SetOverlayMode(m_isOverlay);
                }
            }
        }
    }

    // Shows the transparent overlay window and hides the main one
    void WindowManager::ActivateOverlayMode() {
        m_mainWnd->Hide();
        if (m_uiManager && m_uiManager->GetColorPicker()) {
            m_uiManager->GetColorPicker()->SetVisible(false);
        }

        HWND newHwnd = m_overlayWnd->GetHwnd();
        int screenW, screenH;
        WindowUtils::GetScreenSize(screenW, screenH);
        int overlayH = m_overlayWnd->GetHeight();

        // Position overlay at the bottom of the screen
        SetWindowPos(newHwnd, HWND_TOPMOST, 0, screenH - overlayH, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);

        RecreateGraphicsAndNotify(newHwnd);
    }

    // Shows the main window and hides the overlay
    void WindowManager::DeactivateOverlayMode() {
        m_overlayWnd->Hide();
        m_mainWnd->Show();

        if (m_uiManager && m_uiManager->GetColorPicker()) {
            m_uiManager->GetColorPicker()->SetVisible(true);
        }

        HWND newHwnd = m_mainWnd->GetHwnd();
        SetForegroundWindow(newHwnd);

        RecreateGraphicsAndNotify(newHwnd);
    }
}