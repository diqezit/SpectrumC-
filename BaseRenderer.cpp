// =-=-=-=-=-=-=-=-=-=-=
// BaseRenderer.cpp
// =-=-=-=-=-=-=-=-=-=-=
#include "BaseRenderer.h"
#include <algorithm>

namespace Spectrum {

    BaseRenderer::BaseRenderer()
        : m_quality(RenderQuality::Medium),
        m_primaryColor(Color::FromRGB(33, 150, 243)),
        m_isOverlay(false),
        m_width(0),
        m_height(0),
        m_time(0.0f),
        m_aspectRatio(0.0f), // Default: no fixed aspect ratio
        m_padding(1.0f) {   // Default: full size
    }

    void BaseRenderer::SetQuality(RenderQuality quality) {
        if (m_quality == quality) return;
        m_quality = quality;
        UpdateSettings();
    }

    void BaseRenderer::SetOverlayMode(bool isOverlay) {
        if (m_isOverlay == isOverlay) return;
        m_isOverlay = isOverlay;
        UpdateSettings();
    }

    void BaseRenderer::SetPrimaryColor(const Color& color) {
        m_primaryColor = color;
    }

    void BaseRenderer::OnActivate(int width, int height) {
        SetViewport(width, height);
    }

    void BaseRenderer::Render(
        GraphicsContext& context,
        const SpectrumData& spectrum
    ) {
        if (!IsRenderable(spectrum)) return;

        UpdateTime(FRAME_TIME);
        UpdateAnimation(spectrum, FRAME_TIME);
        DoRender(context, spectrum);
    }

    // Calculates a centered rect, maintaining aspect ratio within the view
    Rect BaseRenderer::CalculatePaddedRect() const {
        float viewWidth = static_cast<float>(m_width);
        float viewHeight = static_cast<float>(m_height);

        // If no aspect ratio is set, return the full view
        if (m_aspectRatio <= 0.0f) {
            return Rect(0.f, 0.f, viewWidth, viewHeight);
        }

        float renderWidth;
        float renderHeight;

        if (viewWidth / viewHeight > m_aspectRatio) {
            // Window is wider than the desired aspect ratio (letterboxing)
            renderHeight = viewHeight * m_padding;
            renderWidth = renderHeight * m_aspectRatio;
        }
        else {
            // Window is taller than the desired aspect ratio (pillarboxing)
            renderWidth = viewWidth * m_padding;
            renderHeight = renderWidth / m_aspectRatio;
        }

        return Rect(
            (viewWidth - renderWidth) / 2.0f,
            (viewHeight - renderHeight) / 2.0f,
            renderWidth,
            renderHeight
        );
    }

    void BaseRenderer::UpdateTime(float deltaTime) {
        m_time += deltaTime;
        if (m_time > TIME_RESET_THRESHOLD) m_time = 0.0f;
    }

    void BaseRenderer::SetViewport(int width, int height) noexcept {
        m_width = width;
        m_height = height;
    }

    bool BaseRenderer::IsRenderable(const SpectrumData& spectrum) const noexcept {
        if (spectrum.empty()) return false;
        if (m_width <= 0 || m_height <= 0) return false;
        return true;
    }

}