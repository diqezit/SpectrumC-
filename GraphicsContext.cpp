// =-=-=-=-=-=-=-=-=-=-=
// GraphicsContext.cpp
// =-=-=-=-=-=-=-=-=-=-=

#include "GraphicsContext.h"
#include "Utils.h"

namespace Spectrum {

    namespace {
        inline D2D1_COLOR_F ToD2DColor(const Color& c) {
            return D2D1::ColorF(c.r, c.g, c.b, c.a);
        }

        inline D2D1_SIZE_U ClientSize(HWND hwnd) {
            RECT rc{};
            GetClientRect(hwnd, &rc);
            return D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
        }

        inline bool CreateGradientStopCollection(
            ID2D1RenderTarget* rt,
            const std::vector<D2D1_GRADIENT_STOP>& stops,
            wrl::ComPtr<ID2D1GradientStopCollection>& out
        ) {
            if (stops.empty() || !rt) return false;
            HRESULT hr = rt->CreateGradientStopCollection(
                stops.data(),
                static_cast<UINT32>(stops.size()),
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                out.GetAddressOf()
            );
            return SUCCEEDED(hr);
        }
    }

    GraphicsContext::GraphicsContext(HWND hwnd)
        : m_hwnd(hwnd), m_width(0), m_height(0) {
        RECT rect;
        if (GetClientRect(hwnd, &rect)) {
            m_width = rect.right - rect.left;
            m_height = rect.bottom - rect.top;
        }
    }

    GraphicsContext::~GraphicsContext() {
        DiscardDeviceResources();
    }

    bool GraphicsContext::Initialize() {
        HRESULT hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            m_d2dFactory.GetAddressOf()
        );
        if (FAILED(hr)) return false;

        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(m_writeFactory.GetAddressOf())
        );
        if (FAILED(hr)) return false;

        return CreateDeviceResources();
    }

    bool GraphicsContext::CreateDeviceResources() {
        if (m_renderTarget) return true;

        HRESULT hr = m_d2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, ClientSize(m_hwnd)),
            m_renderTarget.GetAddressOf()
        );
        if (FAILED(hr)) return false;

        hr = m_renderTarget->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::White),
            m_solidBrush.GetAddressOf()
        );
        return SUCCEEDED(hr);
    }

    void GraphicsContext::DiscardDeviceResources() {
        m_radialBrush.Reset();
        m_linearBrush.Reset();
        m_solidBrush.Reset();
        m_renderTarget.Reset();
    }

    void GraphicsContext::BeginDraw() {
        if (!m_renderTarget) CreateDeviceResources();
        if (m_renderTarget) m_renderTarget->BeginDraw();
    }

    HRESULT GraphicsContext::EndDraw() {
        if (!m_renderTarget) return S_OK;
        HRESULT hr = m_renderTarget->EndDraw();
        if (FAILED(hr)) DiscardDeviceResources();
        return hr;
    }

    void GraphicsContext::Resize(int width, int height) {
        m_width = width;
        m_height = height;
        if (m_renderTarget) {
            HRESULT hr = m_renderTarget->Resize(D2D1::SizeU(width, height));
            if (FAILED(hr)) DiscardDeviceResources();
        }
    }

    void GraphicsContext::Clear(const Color& color) {
        if (m_renderTarget) m_renderTarget->Clear(ToD2DColor(color));
    }

    ID2D1SolidColorBrush* GraphicsContext::GetSolidBrush(const Color& color) {
        if (!m_solidBrush && m_renderTarget) {
            m_renderTarget->CreateSolidColorBrush(
                ToD2DColor(color), m_solidBrush.GetAddressOf()
            );
        }
        if (m_solidBrush) m_solidBrush->SetColor(ToD2DColor(color));
        return m_solidBrush.Get();
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Drawing Primitives
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    void GraphicsContext::DrawRectangle(
        const Rect& rect,
        const Color& color,
        bool filled,
        float strokeWidth
    ) {
        if (!m_renderTarget) return;
        ID2D1SolidColorBrush* b = GetSolidBrush(color);
        if (!b) return;

        D2D1_RECT_F r = D2D1::RectF(rect.x, rect.y, rect.GetRight(), rect.GetBottom());
        if (filled) {
            m_renderTarget->FillRectangle(&r, b);
        }
        else {
            m_renderTarget->DrawRectangle(&r, b, strokeWidth);
        }
    }

    void GraphicsContext::DrawRoundedRectangle(
        const Rect& rect,
        float radius,
        const Color& color,
        bool filled,
        float strokeWidth
    ) {
        if (!m_renderTarget) return;
        ID2D1SolidColorBrush* b = GetSolidBrush(color);
        if (!b) return;

        D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(
            D2D1::RectF(rect.x, rect.y, rect.GetRight(), rect.GetBottom()),
            radius,
            radius
        );
        if (filled) {
            m_renderTarget->FillRoundedRectangle(&rr, b);
        }
        else {
            m_renderTarget->DrawRoundedRectangle(&rr, b, strokeWidth);
        }
    }

    void GraphicsContext::DrawCircle(
        const Point& center,
        float radius,
        const Color& color,
        bool filled,
        float strokeWidth
    ) {
        DrawEllipse(center, radius, radius, color, filled, strokeWidth);
    }

    void GraphicsContext::DrawEllipse(
        const Point& center,
        float radiusX,
        float radiusY,
        const Color& color,
        bool filled,
        float strokeWidth
    ) {
        if (!m_renderTarget) return;
        ID2D1SolidColorBrush* b = GetSolidBrush(color);
        if (!b) return;

        D2D1_ELLIPSE e = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radiusX, radiusY);
        if (filled) {
            m_renderTarget->FillEllipse(&e, b);
        }
        else {
            m_renderTarget->DrawEllipse(&e, b, strokeWidth);
        }
    }

    void GraphicsContext::DrawLine(
        const Point& start,
        const Point& end,
        const Color& color,
        float strokeWidth
    ) {
        if (!m_renderTarget) return;
        ID2D1SolidColorBrush* b = GetSolidBrush(color);
        if (!b) return;
        m_renderTarget->DrawLine(
            D2D1::Point2F(start.x, start.y),
            D2D1::Point2F(end.x, end.y),
            b,
            strokeWidth
        );
    }

    void GraphicsContext::DrawPolyline(
        const std::vector<Point>& points,
        const Color& color,
        float strokeWidth
    ) {
        if (!m_renderTarget || points.size() < 2) return;

        wrl::ComPtr<ID2D1PathGeometry> geo;
        HRESULT hr = m_d2dFactory->CreatePathGeometry(geo.GetAddressOf());
        if (FAILED(hr)) return;

        wrl::ComPtr<ID2D1GeometrySink> sink;
        hr = geo->Open(sink.GetAddressOf());
        if (FAILED(hr)) return;

        sink->BeginFigure(
            D2D1::Point2F(points[0].x, points[0].y),
            D2D1_FIGURE_BEGIN_HOLLOW
        );
        std::vector<D2D1_POINT_2F> d2dPoints;
        for (size_t i = 1; i < points.size(); ++i) {
            d2dPoints.push_back(D2D1::Point2F(points[i].x, points[i].y));
        }
        sink->AddLines(d2dPoints.data(), static_cast<UINT32>(d2dPoints.size()));
        sink->EndFigure(D2D1_FIGURE_END_OPEN);
        hr = sink->Close();
        if (FAILED(hr)) return;

        ID2D1SolidColorBrush* b = GetSolidBrush(color);
        if (!b) return;

        m_renderTarget->DrawGeometry(geo.Get(), b, strokeWidth);
    }

    void GraphicsContext::DrawPolygon(
        const std::vector<Point>& points,
        const Color& color,
        bool filled,
        float strokeWidth
    ) {
        if (!m_renderTarget || points.size() < 3) return;
        wrl::ComPtr<ID2D1PathGeometry> geo;
        HRESULT hr = m_d2dFactory->CreatePathGeometry(geo.GetAddressOf());
        if (FAILED(hr)) return;

        wrl::ComPtr<ID2D1GeometrySink> sink;
        hr = geo->Open(sink.GetAddressOf());
        if (FAILED(hr)) return;

        sink->BeginFigure(
            D2D1::Point2F(points[0].x, points[0].y),
            filled ? D2D1_FIGURE_BEGIN_FILLED : D2D1_FIGURE_BEGIN_HOLLOW
        );
        for (size_t i = 1; i < points.size(); ++i) {
            sink->AddLine(D2D1::Point2F(points[i].x, points[i].y));
        }
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        hr = sink->Close();
        if (FAILED(hr)) return;

        ID2D1SolidColorBrush* b = GetSolidBrush(color);
        if (!b) return;

        if (filled) {
            m_renderTarget->FillGeometry(geo.Get(), b);
        }
        else {
            m_renderTarget->DrawGeometry(geo.Get(), b, strokeWidth);
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Gradients
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    void GraphicsContext::DrawGradientRectangle(
        const Rect& rect,
        const std::vector<D2D1_GRADIENT_STOP>& stops,
        bool horizontal
    ) {
        if (!m_renderTarget) return;

        wrl::ComPtr<ID2D1GradientStopCollection> stopCollection;
        if (!CreateGradientStopCollection(m_renderTarget.Get(), stops, stopCollection)) return;

        D2D1_POINT_2F start = D2D1::Point2F(rect.x, rect.y);
        D2D1_POINT_2F end = horizontal
            ? D2D1::Point2F(rect.GetRight(), rect.y)
            : D2D1::Point2F(rect.x, rect.GetBottom());

        m_linearBrush.Reset();
        HRESULT hr = m_renderTarget->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(start, end),
            stopCollection.Get(),
            m_linearBrush.GetAddressOf()
        );

        if (SUCCEEDED(hr)) {
            D2D1_RECT_F r = D2D1::RectF(rect.x, rect.y, rect.GetRight(), rect.GetBottom());
            m_renderTarget->FillRectangle(&r, m_linearBrush.Get());
        }
    }

    void GraphicsContext::DrawRadialGradient(
        const Point& center,
        float radius,
        const std::vector<D2D1_GRADIENT_STOP>& stops
    ) {
        if (!m_renderTarget) return;

        wrl::ComPtr<ID2D1GradientStopCollection> stopCollection;
        if (!CreateGradientStopCollection(m_renderTarget.Get(), stops, stopCollection)) return;

        m_radialBrush.Reset();
        HRESULT hr = m_renderTarget->CreateRadialGradientBrush(
            D2D1::RadialGradientBrushProperties(
                D2D1::Point2F(center.x, center.y),
                D2D1::Point2F(0, 0),
                radius,
                radius
            ),
            stopCollection.Get(),
            m_radialBrush.GetAddressOf()
        );

        if (SUCCEEDED(hr)) {
            D2D1_ELLIPSE e = D2D1::Ellipse(D2D1::Point2F(center.x, center.y), radius, radius);
            m_renderTarget->FillEllipse(&e, m_radialBrush.Get());
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Text
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    void GraphicsContext::DrawText(
        const std::wstring& text,
        const Point& position,
        const Color& color,
        float fontSize,
        DWRITE_TEXT_ALIGNMENT alignment
    ) {
        if (!m_renderTarget || text.empty() || !m_writeFactory) return;

        wrl::ComPtr<IDWriteTextFormat> tf;
        HRESULT hr = m_writeFactory->CreateTextFormat(
            L"Arial",
            nullptr,
            DWRITE_FONT_WEIGHT_BOLD,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"en-US",
            tf.GetAddressOf()
        );
        if (FAILED(hr)) return;

        tf->SetTextAlignment(alignment);
        tf->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

        ID2D1SolidColorBrush* b = GetSolidBrush(color);
        if (!b) return;

        D2D1_RECT_F layoutRect;
        constexpr float boxWidth = 1000.f;
        if (alignment == DWRITE_TEXT_ALIGNMENT_CENTER) {
            layoutRect = D2D1::RectF(
                position.x - boxWidth / 2.f, position.y - fontSize,
                position.x + boxWidth / 2.f, position.y + fontSize
            );
        }
        else if (alignment == DWRITE_TEXT_ALIGNMENT_LEADING) {
            layoutRect = D2D1::RectF(
                position.x, position.y - fontSize,
                position.x + boxWidth, position.y + fontSize
            );
        }
        else { // Trailing
            layoutRect = D2D1::RectF(
                position.x - boxWidth, position.y - fontSize,
                position.x, position.y + fontSize
            );
        }

        m_renderTarget->DrawTextW(
            text.c_str(),
            static_cast<UINT32>(text.length()),
            tf.Get(),
            &layoutRect,
            b
        );
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Transformations
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    void GraphicsContext::SetTransform(const D2D1_MATRIX_3X2_F& transform) {
        if (m_renderTarget) {
            m_renderTarget->SetTransform(transform);
        }
    }

    void GraphicsContext::ResetTransform() {
        if (m_renderTarget) {
            m_renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        }
    }
}