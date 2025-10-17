// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowManager.cpp: Implementation of the WindowManager class.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "WindowManager.h"
#include "ControllerCore.h"
#include "WindowHelper.h"
#include "EventBus.h"

namespace Spectrum {

    WindowManager::WindowManager(
        HINSTANCE hInstance,
        ControllerCore* controller,
        EventBus* bus
    ) : m_hInstance(hInstance),
        m_controller(controller),
        m_isOverlay(false)
    {
        bus->Subscribe(InputAction::ToggleOverlay, [this]() {
            this->ToggleOverlay();
            });

        bus->Subscribe(InputAction::Exit, [this]() {
            // In overlay mode ESC reverts to main window
            // In main window it closes application
            if (this->IsOverlayMode()) {
                this->ToggleOverlay();
            }
            else if (m_controller) {
                m_controller->OnClose();
            }
            });
    }

    WindowManager::~WindowManager() = default;

    bool WindowManager::Initialize() {
        if (!InitializeMainWindow()) {
            return false;
        }
        if (!InitializeOverlayWindow()) {
            return false;
        }
        if (!RecreateGraphicsAndNotify(m_mainWnd->GetHwnd())) {
            return false;
        }
        if (!InitializeUI()) {
            return false;
        }

        WindowUtils::CenterOnScreen(m_mainWnd->GetHwnd());
        m_mainWnd->Show();
        return true;
    }

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

    bool WindowManager::InitializeUI() {
        m_colorPicker = std::make_unique<ColorPicker>(
            Point{ 20.0f, 20.0f }, 40.0f
        );
        if (!m_colorPicker->Initialize(*m_graphics)) {
            return false;
        }

        m_colorPicker->SetOnColorSelectedCallback([this](const Color& color) {
            m_controller->SetPrimaryColor(color);
            });
        return true;
    }

    void WindowManager::ProcessMessages() {
        if (m_mainWnd && m_mainWnd->IsRunning()) {
            m_mainWnd->ProcessMessages();
        }
    }

    bool WindowManager::IsRunning() const {
        return m_mainWnd && m_mainWnd->IsRunning();
    }

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

    bool WindowManager::RecreateGraphicsAndNotify(HWND hwnd) {
        if (!hwnd) return false;

        m_graphics.reset();
        m_graphics = std::make_unique<GraphicsContext>(hwnd);
        if (!m_graphics->Initialize()) {
            return false;
        }

        if (m_colorPicker) {
            m_colorPicker->RecreateResources(*m_graphics);
        }

        RECT rc;
        GetClientRect(hwnd, &rc);
        m_controller->OnResize(rc.right - rc.left, rc.bottom - rc.top);
        return true;
    }

    void WindowManager::ToggleOverlay() {
        m_isOverlay = !m_isOverlay;

        if (m_isOverlay) {
            ActivateOverlayMode();
        }
        else {
            DeactivateOverlayMode();
        }
    }

    void WindowManager::ActivateOverlayMode() {
        m_mainWnd->Hide();
        if (m_colorPicker) {
            m_colorPicker->SetVisible(false);
        }

        HWND newHwnd = m_overlayWnd->GetHwnd();

        int screenW, screenH;
        WindowUtils::GetScreenSize(screenW, screenH);
        int overlayH = m_overlayWnd->GetHeight();
        int yPos = screenH - overlayH;

        SetWindowPos(
            newHwnd, HWND_TOPMOST, 0, yPos, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW
        );

        RecreateGraphicsAndNotify(newHwnd);
    }

    void WindowManager::DeactivateOverlayMode() {
        m_overlayWnd->Hide();
        m_mainWnd->Show();
        if (m_colorPicker) {
            m_colorPicker->SetVisible(true);
        }

        HWND newHwnd = m_mainWnd->GetHwnd();
        SetForegroundWindow(m_mainWnd->GetHwnd());

        RecreateGraphicsAndNotify(newHwnd);
    }
}