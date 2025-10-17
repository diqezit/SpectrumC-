// MainWindow.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// MainWindow.cpp: Implementation of the MainWindow class.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "MainWindow.h"
#include "WindowHelper.h"
#include "ControllerCore.h"
#include "WindowManager.h"

namespace Spectrum {

    MainWindow::MainWindow(HINSTANCE hInstance)
        : m_hInstance(hInstance),
        m_hwnd(nullptr),
        m_running(false),
        m_isOverlay(false),
        m_width(0),
        m_height(0) {
    }

    MainWindow::~MainWindow() {
        if (m_hwnd) {
            DestroyWindow(m_hwnd);
        }
        if (!m_className.empty()) {
            UnregisterClassW(m_className.c_str(), m_hInstance);
        }
    }

    bool MainWindow::Initialize(
        const std::wstring& title,
        int width,
        int height,
        bool isOverlay,
        void* userPtr
    ) {
        m_width = width;
        m_height = height;
        m_isOverlay = isOverlay;
        m_className = isOverlay ? L"SpectrumOverlayClass" : L"SpectrumMainClass";

        if (!Register()) {
            return false;
        }

        if (!CreateWindowInstance(title, width, height, userPtr)) {
            return false;
        }

        ApplyStyles();
        m_running = true;
        return true;
    }

    bool MainWindow::Register() {
        return WindowUtils::RegisterWindowClass(
            m_hInstance, m_className.c_str(), WndProc, m_isOverlay
        );
    }

    bool MainWindow::CreateWindowInstance(
        const std::wstring& title,
        int width,
        int height,
        void* userPtr
    ) {
        auto styles = WindowUtils::MakeStyles(m_isOverlay);
        RECT rc = { 0, 0, width, height };
        WindowUtils::AdjustRectIfNeeded(rc, styles, m_isOverlay);

        int w = m_isOverlay ? width : rc.right - rc.left;
        int h = m_isOverlay ? height : rc.bottom - rc.top;
        int x = m_isOverlay ? 0 : CW_USEDEFAULT;
        int y = m_isOverlay ? 0 : CW_USEDEFAULT;

        m_hwnd = WindowUtils::CreateWindowWithStyles(
            m_hInstance,
            m_className.c_str(),
            title.c_str(),
            styles,
            x, y, w, h,
            userPtr
        );

        return m_hwnd != nullptr;
    }

    void MainWindow::ApplyStyles() {
        if (m_isOverlay) {
            WindowUtils::ApplyOverlay(m_hwnd);
        }
    }

    void MainWindow::ProcessMessages() {
        MSG msg = {};
        while (m_running && PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void MainWindow::Show(int cmdShow) const {
        ShowWindow(m_hwnd, cmdShow);
        UpdateWindow(m_hwnd);
    }

    void MainWindow::Hide() const {
        ShowWindow(m_hwnd, SW_HIDE);
    }

    void MainWindow::Close() {
        if (m_running) {
            PostMessage(m_hwnd, WM_CLOSE, 0, 0);
        }
    }

    LRESULT CALLBACK MainWindow::WndProc(
        HWND hwnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    ) {
        WindowManager* wm = nullptr;
        if (msg == WM_NCCREATE) {
            auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            wm = reinterpret_cast<WindowManager*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wm));
        }
        else {
            wm = reinterpret_cast<WindowManager*>(
                GetWindowLongPtr(hwnd, GWLP_USERDATA)
                );
        }

        if (wm && wm->GetController()) {
            return wm->GetController()->HandleWindowMessage(
                hwnd, msg, wParam, lParam
            );
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}