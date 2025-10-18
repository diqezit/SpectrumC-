// =-=-=-=-=-=-=-=-=-=-=
// GraphicsContext.h
// =-=-=-=-=-=-=-=-=-=-=

#ifndef SPECTRUM_CPP_GRAPHICS_CONTEXT_H
#define SPECTRUM_CPP_GRAPHICS_CONTEXT_H

#include "Common.h"

namespace Spectrum {

    class GraphicsContext {
    public:
        explicit GraphicsContext(HWND hwnd);
        ~GraphicsContext();

        bool Initialize();
        void BeginDraw();
        HRESULT EndDraw();
        void Resize(int width, int height);
        void Clear(const Color& color);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Drawing Primitives
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void DrawRectangle(
            const Rect& rect,
            const Color& color,
            bool filled = true,
            float strokeWidth = 1.0f
        );
        void DrawRoundedRectangle(
            const Rect& rect,
            float radius,
            const Color& color,
            bool filled = true,
            float strokeWidth = 1.0f
        );
        void DrawCircle(
            const Point& center,
            float radius,
            const Color& color,
            bool filled = true,
            float strokeWidth = 1.0f
        );
        void DrawEllipse(
            const Point& center,
            float radiusX,
            float radiusY,
            const Color& color,
            bool filled = true,
            float strokeWidth = 1.0f
        );
        void DrawLine(
            const Point& start,
            const Point& end,
            const Color& color,
            float strokeWidth = 1.0f
        );
        void DrawPolyline(
            const std::vector<Point>& points,
            const Color& color,
            float strokeWidth = 1.0f
        );
        void DrawPolygon(
            const std::vector<Point>& points,
            const Color& color,
            bool filled = true,
            float strokeWidth = 1.0f
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Gradients
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void DrawGradientRectangle(
            const Rect& rect,
            const std::vector<D2D1_GRADIENT_STOP>& stops,
            bool horizontal = true
        );
        void DrawRadialGradient(
            const Point& center,
            float radius,
            const std::vector<D2D1_GRADIENT_STOP>& stops
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Text
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void DrawText(
            const std::wstring& text,
            const Point& position,
            const Color& color,
            float fontSize = 12.0f,
            DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Transformations
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void SetTransform(const D2D1_MATRIX_3X2_F& transform);
        void ResetTransform();

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Getters
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        ID2D1HwndRenderTarget* GetRenderTarget() const noexcept {
            return m_renderTarget.Get();
        }
        int GetWidth() const noexcept { return m_width; }
        int GetHeight() const noexcept { return m_height; }

    private:
        bool CreateDeviceResources();
        void DiscardDeviceResources();
        ID2D1SolidColorBrush* GetSolidBrush(const Color& color);

        HWND m_hwnd;
        int  m_width;
        int  m_height;

        wrl::ComPtr<ID2D1Factory>          m_d2dFactory;
        wrl::ComPtr<ID2D1HwndRenderTarget> m_renderTarget;
        wrl::ComPtr<ID2D1SolidColorBrush>  m_solidBrush;
        wrl::ComPtr<IDWriteFactory>        m_writeFactory;

        wrl::ComPtr<ID2D1LinearGradientBrush> m_linearBrush;
        wrl::ComPtr<ID2D1RadialGradientBrush> m_radialBrush;
    };

}

#endif