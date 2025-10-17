// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// WindowManager.h: Manages main and overlay windows.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef SPECTRUM_CPP_WINDOW_MANAGER_H
#define SPECTRUM_CPP_WINDOW_MANAGER_H

#include "Common.h"
#include "MainWindow.h"
#include "GraphicsContext.h"
#include "ColorPicker.h"

namespace Spectrum {

    class ControllerCore;
    class EventBus;

    class WindowManager {
    public:
        explicit WindowManager(HINSTANCE hInstance, ControllerCore* controller, EventBus* bus);
        ~WindowManager();

        bool Initialize();
        void ProcessMessages();

        void ToggleOverlay();

        bool IsRunning() const;
        bool IsOverlayMode() const { return m_isOverlay; }
        bool IsActive() const;

        GraphicsContext* GetGraphics() const { return m_graphics.get(); }
        ColorPicker* GetColorPicker() const { return m_colorPicker.get(); }
        HWND GetCurrentHwnd() const;
        ControllerCore* GetController() const { return m_controller; }
        MainWindow* GetMainWindow() const { return m_mainWnd.get(); }

        bool RecreateGraphicsAndNotify(HWND hwnd);

    private:
        bool InitializeMainWindow();
        bool InitializeOverlayWindow();
        bool InitializeUI();

        void ActivateOverlayMode();
        void DeactivateOverlayMode();

        HINSTANCE m_hInstance;
        ControllerCore* m_controller;
        bool m_isOverlay;

        std::unique_ptr<MainWindow> m_mainWnd;
        std::unique_ptr<MainWindow> m_overlayWnd;

        std::unique_ptr<GraphicsContext> m_graphics;
        std::unique_ptr<ColorPicker> m_colorPicker;
    };

}

#endif