// =-=-=-=-=-=-=-=-=-=-=
// CircularWaveRenderer.cpp
// =-=-=-=-=-=-=-=-=-=-=

#include "CircularWaveRenderer.h"
#include "Utils.h"
#include "RenderUtils.h"

namespace Spectrum {

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Class Implementation
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    CircularWaveRenderer::CircularWaveRenderer()
        : m_angle(0.0f), m_waveTime(0.0f) {
        m_primaryColor = Color::FromRGB(0, 150, 255);
        UpdateSettings();
    }

    void CircularWaveRenderer::OnActivate(int width, int height) {
        BaseRenderer::OnActivate(width, height);
        m_circlePoints.clear();
    }

    void CircularWaveRenderer::UpdateSettings() {
        // Shared settings
        constexpr float centerRadius = 30.0f;
        constexpr float maxRadiusFactor = 0.45f;
        constexpr float minStroke = 1.5f;
        constexpr float waveInfluence = 1.0f;
        constexpr float glowThreshold = 0.5f;
        constexpr float glowFactor = 0.7f;
        constexpr float glowWidthFactor = 1.5f;
        constexpr float rotationIntensityFactor = 0.3f;
        constexpr float wavePhaseOffset = 0.1f;
        constexpr float strokeClampFactor = 6.0f;
        constexpr float minMagnitudeThreshold = 0.01f;

        if (m_isOverlay) {
            switch (m_quality) {
            case RenderQuality::Low:
                m_settings = {
                    16, false, 4.f, 12, 0.4f, 1.5f,
                    centerRadius, maxRadiusFactor, minStroke, waveInfluence,
                    glowThreshold, glowFactor, glowWidthFactor,
                    rotationIntensityFactor, wavePhaseOffset,
                    strokeClampFactor, minMagnitudeThreshold
                };
                break;
            case RenderQuality::High:
                m_settings = {
                    48, true, 6.f, 20, 0.4f, 1.5f,
                    centerRadius, maxRadiusFactor, minStroke, waveInfluence,
                    glowThreshold, glowFactor, glowWidthFactor,
                    rotationIntensityFactor, wavePhaseOffset,
                    strokeClampFactor, minMagnitudeThreshold
                };
                break;
            default: // Medium
                m_settings = {
                    32, true, 5.f, 16, 0.4f, 1.5f,
                    centerRadius, maxRadiusFactor, minStroke, waveInfluence,
                    glowThreshold, glowFactor, glowWidthFactor,
                    rotationIntensityFactor, wavePhaseOffset,
                    strokeClampFactor, minMagnitudeThreshold
                };
                break;
            }
        }
        else {
            switch (m_quality) {
            case RenderQuality::Low:
                m_settings = {
                    32, false, 6.f, 16, 0.5f, 2.0f,
                    centerRadius, maxRadiusFactor, minStroke, waveInfluence,
                    glowThreshold, glowFactor, glowWidthFactor,
                    rotationIntensityFactor, wavePhaseOffset,
                    strokeClampFactor, minMagnitudeThreshold
                };
                break;
            case RenderQuality::High:
                m_settings = {
                    128, true, 8.f, 32, 0.5f, 2.0f,
                    centerRadius, maxRadiusFactor, minStroke, waveInfluence,
                    glowThreshold, glowFactor, glowWidthFactor,
                    rotationIntensityFactor, wavePhaseOffset,
                    strokeClampFactor, minMagnitudeThreshold
                };
                break;
            default: // Medium
                m_settings = {
                    64, true, 7.f, 24, 0.5f, 2.0f,
                    centerRadius, maxRadiusFactor, minStroke, waveInfluence,
                    glowThreshold, glowFactor, glowWidthFactor,
                    rotationIntensityFactor, wavePhaseOffset,
                    strokeClampFactor, minMagnitudeThreshold
                };
                break;
            }
        }
        m_circlePoints.clear();
    }

    void CircularWaveRenderer::UpdateAnimation(
        const SpectrumData& spectrum,
        float deltaTime
    ) {
        float avgIntensity = RenderUtils::GetAverageMagnitude(spectrum);

        m_angle += m_settings.rotationSpeed
            * (1.0f + avgIntensity * m_settings.rotationIntensityFactor)
            * deltaTime;
        if (m_angle > TWO_PI) {
            m_angle -= TWO_PI;
        }

        m_waveTime += m_settings.waveSpeed * deltaTime;
    }

    void CircularWaveRenderer::EnsureCirclePoints() {
        if (!m_circlePoints.empty()) return;

        m_circlePoints.reserve(m_settings.pointsPerCircle + 1);
        const float step = TWO_PI / m_settings.pointsPerCircle;

        for (int i = 0; i <= m_settings.pointsPerCircle; ++i) {
            float a = i * step;
            m_circlePoints.emplace_back(std::cos(a), std::sin(a));
        }
    }

    void CircularWaveRenderer::DoRender(
        GraphicsContext& context,
        const SpectrumData& spectrum
    ) {
        EnsureCirclePoints();

        const Point center = { m_width * 0.5f, m_height * 0.5f };
        const float maxRadius = std::min(m_width, m_height)
            * m_settings.maxRadiusFactor;

        int ringCount = std::min(
            static_cast<int>(spectrum.size()),
            m_settings.maxRings
        );
        if (ringCount == 0) return;

        float ringStep = (maxRadius - m_settings.centerRadius) / ringCount;

        for (int i = ringCount - 1; i >= 0; i--) {
            RenderRing(context, spectrum, i, ringCount, ringStep, center, maxRadius);
        }
    }

    void CircularWaveRenderer::RenderRing(
        GraphicsContext& context,
        const SpectrumData& spectrum,
        int index,
        int totalRings,
        float ringStep,
        const Point& center,
        float maxRadius
    ) {
        float magnitude = GetRingMagnitude(spectrum, index, totalRings);
        if (magnitude < m_settings.minMagnitudeThreshold) return;

        float radius = CalculateRingRadius(index, ringStep, magnitude);
        if (radius <= 0 || radius > maxRadius) return;

        float distanceFactor = 1.0f - radius / maxRadius;
        float alpha = Utils::Saturate(magnitude * 1.5f * distanceFactor);
        float strokeWidth = CalculateStrokeWidth(magnitude);

        if (m_settings.useGlow && magnitude > m_settings.glowThreshold) {
            RenderGlowLayer(context, center, radius, alpha, strokeWidth);
        }

        RenderMainRing(context, center, radius, alpha, strokeWidth);
    }

    void CircularWaveRenderer::RenderGlowLayer(
        GraphicsContext& context,
        const Point& center,
        float radius,
        float alpha,
        float strokeWidth
    ) {
        Color glowColor = m_primaryColor;
        glowColor.a = alpha * m_settings.glowFactor;
        float glowWidth = strokeWidth * m_settings.glowWidthFactor;

        DrawCirclePath(context, center, radius, glowColor, glowWidth);
    }

    void CircularWaveRenderer::RenderMainRing(
        GraphicsContext& context,
        const Point& center,
        float radius,
        float alpha,
        float strokeWidth
    ) {
        Color ringColor = m_primaryColor;
        ringColor.a = alpha;
        DrawCirclePath(context, center, radius, ringColor, strokeWidth);
    }

    void CircularWaveRenderer::DrawCirclePath(
        GraphicsContext& context,
        const Point& center,
        float radius,
        const Color& color,
        float strokeWidth
    ) {
        if (m_circlePoints.empty()) return;

        std::vector<Point> path;
        path.reserve(m_circlePoints.size());
        for (const auto& p : m_circlePoints) {
            path.push_back(center + p * radius);
        }
        context.DrawPolyline(path, color, strokeWidth);
    }


    float CircularWaveRenderer::CalculateRingRadius(
        int index,
        float ringStep,
        float magnitude
    ) const {
        float baseRadius = m_settings.centerRadius + index * ringStep;
        float waveOffset = std::sin(
            m_waveTime + index * m_settings.wavePhaseOffset + m_angle
        ) * magnitude * ringStep * m_settings.waveInfluence;

        return baseRadius + waveOffset;
    }

    float CircularWaveRenderer::CalculateStrokeWidth(float magnitude) const {
        return Utils::Clamp(
            m_settings.minStroke + magnitude * m_settings.strokeClampFactor,
            m_settings.minStroke,
            m_settings.maxStroke
        );
    }

    float CircularWaveRenderer::GetRingMagnitude(
        const SpectrumData& spectrum,
        int ringIndex,
        int ringCount
    ) {
        if (spectrum.empty() || ringCount == 0) return 0.0f;

        size_t start = static_cast<size_t>(ringIndex) * spectrum.size() / ringCount;
        size_t end = std::min(
            (static_cast<size_t>(ringIndex) + 1) * spectrum.size() / ringCount,
            spectrum.size()
        );

        if (start >= end) return 0.0f;

        float sum = 0.0f;
        for (size_t i = start; i < end; ++i) {
            sum += spectrum[i];
        }
        return sum / (end - start);
    }

}