// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// BaseRenderer.cpp: Implementation of the BaseRenderer class.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "BaseRenderer.h"
#include <algorithm>

namespace Spectrum {

    BaseRenderer::BaseRenderer()
        : m_quality(RenderQuality::Medium),
        m_primaryColor(Color::FromRGB(33, 150, 243)),
        m_backgroundColor(Color::FromRGB(13, 13, 26)),
        m_width(0),
        m_height(0),
        m_time(0.0f) {
    }

    void BaseRenderer::SetQuality(RenderQuality quality) {
        m_quality = quality;
        UpdateSettings();
    }

    void BaseRenderer::SetPrimaryColor(const Color& color) {
        m_primaryColor = color;
    }

    void BaseRenderer::SetBackgroundColor(const Color& color) {
        m_backgroundColor = color;
    }

    void BaseRenderer::OnActivate(int width, int height) {
        SetViewport(width, height);
    }

    void BaseRenderer::Render(GraphicsContext& context, const SpectrumData& spectrum) {
        if (!IsRenderable(spectrum)) return;

        UpdateTime(FRAME_TIME);
        UpdateAnimation(spectrum, FRAME_TIME);
        DoRender(context, spectrum);
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