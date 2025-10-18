// =-=-=-=-=-=-=-=-=-=-=
// WindowManager.h
// =-=-=-=-=-=-=-=-=-=-=

#ifndef SPECTRUM_CPP_WINDOW_MANAGER_H
#define SPECTRUM_CPP_WINDOW_MANAGER_H

#include "Common.h"
#include "MainWindow.h"

namespace Spectrum {

    class ControllerCore;
    class EventBus;
    class GraphicsContext;
    class UIManager;

    class WindowManager {
    public:
        explicit WindowManager(HINSTANCE hInstance, ControllerCore* controller, EventBus* bus);
        ~WindowManager();

        bool Initialize();
        void ProcessMessages();

        // Handles switching between main window and overlay
        void ToggleOverlay();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // State & Getters
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        bool IsRunning() const;
        bool IsOverlayMode() const { return m_isOverlay; }
        bool IsActive() const;

        GraphicsContext* GetGraphics() const { return m_graphics.get(); }
        UIManager* GetUIManager() const { return m_uiManager.get(); }
        HWND GetCurrentHwnd() const;
        ControllerCore* GetController() const { return m_controller; }
        MainWindow* GetMainWindow() const { return m_mainWnd.get(); }

        // Recreates D2D resources on window change or device loss
        bool RecreateGraphicsAndNotify(HWND hwnd);

    private:
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Initialization
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        bool InitializeMainWindow();
        bool InitializeOverlayWindow();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Overlay Management
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void ActivateOverlayMode();
        void DeactivateOverlayMode();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Member State
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        HINSTANCE m_hInstance;
        ControllerCore* m_controller;
        bool m_isOverlay;

        std::unique_ptr<MainWindow> m_mainWnd;
        std::unique_ptr<MainWindow> m_overlayWnd;

        std::unique_ptr<GraphicsContext> m_graphics;
        std::unique_ptr<UIManager> m_uiManager;
    };

}

#endif