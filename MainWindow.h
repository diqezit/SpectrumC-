#ifndef SPECTRUM_CPP_MAINWINDOW_H
#define SPECTRUM_CPP_MAINWINDOW_H

#include "Common.h"

namespace Spectrum {

    class MainWindow {
    public:
        explicit MainWindow(HINSTANCE hInstance);
        ~MainWindow();

        bool Initialize(
            const std::wstring& title,
            int width,
            int height,
            bool isOverlay,
            void* userPtr
        );

        void ProcessMessages();
        void Show(int cmdShow = SW_SHOW) const;
        void Hide() const;
        void Close();

        HWND GetHwnd() const { return m_hwnd; }
        bool IsRunning() const { return m_running; }
        void SetRunning(bool running) { m_running = running; }

        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

    private:
        bool Register();
        bool CreateWindowInstance(
            const std::wstring& title,
            int width,
            int height,
            void* userPtr
        );
        void ApplyStyles();

        static LRESULT CALLBACK WndProc(
            HWND hwnd,
            UINT msg,
            WPARAM wParam,
            LPARAM lParam
        );

        HINSTANCE m_hInstance;
        HWND m_hwnd;
        std::wstring m_className;
        std::atomic<bool> m_running;
        bool m_isOverlay;
        int m_width;
        int m_height;
    };

}

#endif