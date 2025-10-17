// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// ControllerCore.h: The main application controller.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef SPECTRUM_CPP_CONTROLLER_CORE_H
#define SPECTRUM_CPP_CONTROLLER_CORE_H

#include "Common.h"
#include "Utils.h"
#include "EventBus.h"
#include <memory>
#include <vector>

namespace Spectrum {

    class WindowManager;
    class AudioManager;
    class RendererManager;
    class InputManager;

    class ControllerCore {
    public:
        explicit ControllerCore(HINSTANCE hInstance);
        ~ControllerCore();

        bool Initialize();
        void Run();

        LRESULT HandleWindowMessage(
            HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam
        );

        void OnResize(int width, int height);
        void SetPrimaryColor(const Color& color);
        void OnClose();

    private:
        bool InitializeManagers();
        void PrintWelcomeMessage();

        void MainLoop();
        void ProcessInput();
        void Update(float deltaTime);
        void Render();

        LRESULT HandleMouseMessage(UINT msg, LPARAM lParam);

    private:
        HINSTANCE m_hInstance;

        std::unique_ptr<WindowManager> m_windowManager;
        std::unique_ptr<AudioManager> m_audioManager;
        std::unique_ptr<RendererManager> m_rendererManager;
        std::unique_ptr<InputManager> m_inputManager;
        std::unique_ptr<EventBus> m_eventBus;

        Utils::Timer m_timer;
        std::vector<InputAction> m_actions;
    };

}

#endif