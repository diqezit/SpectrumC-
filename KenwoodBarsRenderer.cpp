// =-=-=-=-=-=-=-=-=-=-=
// KenwoodBarsRenderer.cpp
// =-=-=-=-=-=-=-=-=-=-=

#include "KenwoodBarsRenderer.h"
#include "RenderUtils.h"
#include "Utils.h"

namespace Spectrum {

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Constants
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    namespace {
        // Peak behavior
        constexpr float PEAK_FALL_SPEED = 0.25f;
        constexpr float PEAK_HEIGHT = 3.0f;
        constexpr float PEAK_HEIGHT_OVERLAY = 2.0f;
        constexpr float PEAK_HOLD_TIME_S = 0.3f; // 300ms

        // Bar geometry
        constexpr float MIN_BAR_HEIGHT = 2.0f;
        constexpr float MIN_MAGNITUDE_FOR_RENDER = 0.01f;
        constexpr float CORNER_RADIUS_RATIO = 0.25f;
        constexpr float CORNER_RADIUS_RATIO_OVERLAY = 0.2f;

        // Outline styles
        constexpr float OUTLINE_WIDTH = 1.5f;
        constexpr float OUTLINE_WIDTH_OVERLAY = 1.0f;
        constexpr float OUTLINE_ALPHA = 0.5f;
        constexpr float OUTLINE_ALPHA_OVERLAY = 0.35f;
        constexpr float PEAK_OUTLINE_ALPHA = 0.7f;
        constexpr float PEAK_OUTLINE_ALPHA_OVERLAY = 0.5f;

        // Gradient boost
        constexpr float GRADIENT_INTENSITY_BOOST = 1.1f;
        constexpr float GRADIENT_INTENSITY_BOOST_OVERLAY = 0.95f;

        // Base gradient colors and positions for the bars
        const std::vector<D2D1_GRADIENT_STOP> BAR_GRADIENT_STOPS_BASE = {
            { 0.00f, D2D1::ColorF(0.f, 240 / 255.f, 120 / 255.f) },
            { 0.55f, D2D1::ColorF(0.f, 255 / 255.f, 0 / 255.f) },
            { 0.55f, D2D1::ColorF(255 / 255.f, 235 / 255.f, 0.f) },
            { 0.80f, D2D1::ColorF(255 / 255.f, 185 / 255.f, 0.f) },
            { 0.80f, D2D1::ColorF(255 / 255.f, 85 / 255.f, 0.f) },
            { 1.00f, D2D1::ColorF(255 / 255.f, 35 / 255.f, 0.f) }
        };

        const Color PEAK_COLOR = Color::White();
        const Color PEAK_OUTLINE_COLOR = Color(1.0f, 1.0f, 1.0f, 0.8f);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Class Implementation
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    KenwoodBarsRenderer::KenwoodBarsRenderer() {
        UpdateSettings();
    }

    void KenwoodBarsRenderer::UpdateSettings() {
        switch (m_quality) {
        case RenderQuality::Low:
            m_currentSettings = { true, false, false, false };
            break;
        case RenderQuality::High:
            m_currentSettings = { true, true, true, true };
            break;
        case RenderQuality::Medium:
        default:
            m_currentSettings = { true, true, true, true };
            break;
        }
    }

    void KenwoodBarsRenderer::UpdateAnimation(
        const SpectrumData& spectrum,
        float deltaTime
    ) {
        EnsurePeakArraySize(spectrum.size());
        for (size_t i = 0; i < spectrum.size(); ++i) {
            UpdatePeak(i, spectrum[i], deltaTime);
        }
    }

    void KenwoodBarsRenderer::DoRender(
        GraphicsContext& context,
        const SpectrumData& spectrum
    ) {
        auto layout = RenderUtils::ComputeBarLayout(spectrum.size(), 2.0f, m_width);
        if (layout.barWidth <= 0) return;

        RenderData data = CalculateRenderData(spectrum, layout);

        RenderMainLayer(context, data, layout);
        RenderPeakLayer(context, data, layout);

        if (m_currentSettings.useOutline) {
            RenderOutlineLayer(context, data, layout);
        }
        if (m_currentSettings.useEnhancedPeaks) {
            RenderPeakEnhancementLayer(context, data);
        }
    }

    KenwoodBarsRenderer::RenderData KenwoodBarsRenderer::CalculateRenderData(
        const SpectrumData& spectrum,
        const RenderUtils::BarLayout& layout
    ) {
        RenderData data;
        data.bars.reserve(spectrum.size());
        data.peaks.reserve(spectrum.size());

        float peakHeight = m_isOverlay ? PEAK_HEIGHT_OVERLAY : PEAK_HEIGHT;

        for (size_t i = 0; i < spectrum.size(); ++i) {
            float magnitude = std::max(spectrum[i], 0.0f);

            if (magnitude > MIN_MAGNITUDE_FOR_RENDER) {
                float barHeight = RenderUtils::MagnitudeToHeight(magnitude, m_height);
                if (barHeight < MIN_BAR_HEIGHT) barHeight = MIN_BAR_HEIGHT;

                Rect barRect(
                    i * layout.totalBarWidth,
                    m_height - barHeight,
                    layout.barWidth,
                    barHeight
                );
                data.bars.push_back({ barRect, magnitude });
            }

            float peakValue = GetPeakValue(i);
            if (peakValue > MIN_MAGNITUDE_FOR_RENDER) {
                float peakY = m_height - (peakValue * m_height);
                Rect peakRect(
                    i * layout.totalBarWidth,
                    std::max(0.0f, peakY - peakHeight),
                    layout.barWidth,
                    peakHeight
                );
                data.peaks.push_back({ peakRect });
            }
        }
        return data;
    }

    void KenwoodBarsRenderer::RenderMainLayer(
        GraphicsContext& context,
        const RenderData& data,
        const RenderUtils::BarLayout& layout
    ) {
        if (data.bars.empty()) return;

        float cornerRadius = m_currentSettings.useRoundCorners
            ? layout.barWidth * (m_isOverlay ? CORNER_RADIUS_RATIO_OVERLAY : CORNER_RADIUS_RATIO)
            : 0.0f;

        if (m_currentSettings.useGradient) {
            float intensityBoost = m_isOverlay
                ? GRADIENT_INTENSITY_BOOST_OVERLAY
                : GRADIENT_INTENSITY_BOOST;

            std::vector<D2D1_GRADIENT_STOP> adjustedStops;
            adjustedStops.reserve(BAR_GRADIENT_STOPS_BASE.size());

            for (const auto& stop : BAR_GRADIENT_STOPS_BASE) {
                D2D1_GRADIENT_STOP newStop = stop;
                newStop.color.r = std::min(1.0f, stop.color.r * intensityBoost);
                newStop.color.g = std::min(1.0f, stop.color.g * intensityBoost);
                newStop.color.b = std::min(1.0f, stop.color.b * intensityBoost);
                adjustedStops.push_back(newStop);
            }

            for (const auto& bar : data.bars) {
                context.DrawGradientRectangle(bar.rect, adjustedStops, false);
            }
        }
        else {
            // Fallback to a solid color if gradients are off
            Color solidColor = Color::FromRGB(0, 240, 120);
            for (const auto& bar : data.bars) {
                if (cornerRadius > 0) {
                    context.DrawRoundedRectangle(bar.rect, cornerRadius, solidColor, true);
                }
                else {
                    context.DrawRectangle(bar.rect, solidColor, true);
                }
            }
        }
    }

    void KenwoodBarsRenderer::RenderOutlineLayer(
        GraphicsContext& context,
        const RenderData& data,
        const RenderUtils::BarLayout& layout
    ) {
        float outlineWidth = m_isOverlay ? OUTLINE_WIDTH_OVERLAY : OUTLINE_WIDTH;
        float baseAlpha = m_isOverlay ? OUTLINE_ALPHA_OVERLAY : OUTLINE_ALPHA;
        float cornerRadius = m_currentSettings.useRoundCorners
            ? layout.barWidth * (m_isOverlay ? CORNER_RADIUS_RATIO_OVERLAY : CORNER_RADIUS_RATIO)
            : 0.0f;

        for (const auto& bar : data.bars) {
            float alpha = Utils::Saturate(bar.magnitude * 1.5f) * baseAlpha;
            Color outlineColor(1.0f, 1.0f, 1.0f, alpha);

            if (cornerRadius > 0.0f) {
                context.DrawRoundedRectangle(bar.rect, cornerRadius, outlineColor, false, outlineWidth);
            }
            else {
                context.DrawRectangle(bar.rect, outlineColor, false, outlineWidth);
            }
        }
    }

    void KenwoodBarsRenderer::RenderPeakLayer(
        GraphicsContext& context,
        const RenderData& data,
        const RenderUtils::BarLayout& layout
    ) {
        if (data.peaks.empty()) return;

        float cornerRadius = m_currentSettings.useRoundCorners
            ? (layout.barWidth * (m_isOverlay ? CORNER_RADIUS_RATIO_OVERLAY : CORNER_RADIUS_RATIO)) * 0.5f
            : 0.0f;

        for (const auto& peak : data.peaks) {
            if (cornerRadius > 0.0f) {
                context.DrawRoundedRectangle(peak.rect, cornerRadius, PEAK_COLOR, true);
            }
            else {
                context.DrawRectangle(peak.rect, PEAK_COLOR, true);
            }
        }
    }

    void KenwoodBarsRenderer::RenderPeakEnhancementLayer(
        GraphicsContext& context,
        const RenderData& data
    ) {
        float outlineWidth = (m_isOverlay ? OUTLINE_WIDTH_OVERLAY : OUTLINE_WIDTH) * 0.75f;
        float baseAlpha = m_isOverlay ? PEAK_OUTLINE_ALPHA_OVERLAY : PEAK_OUTLINE_ALPHA;
        Color outlineColor = PEAK_OUTLINE_COLOR;
        outlineColor.a = baseAlpha;

        for (const auto& peak : data.peaks) {
            context.DrawLine(
                { peak.rect.x, peak.rect.y },
                { peak.rect.GetRight(), peak.rect.y },
                outlineColor,
                outlineWidth
            );
            context.DrawLine(
                { peak.rect.x, peak.rect.GetBottom() },
                { peak.rect.GetRight(), peak.rect.GetBottom() },
                outlineColor,
                outlineWidth
            );
        }
    }

    void KenwoodBarsRenderer::EnsurePeakArraySize(size_t size) {
        if (m_peaks.size() != size) {
            m_peaks.assign(size, 0.0f);
            m_peakTimers.assign(size, 0.0f);
        }
    }

    void KenwoodBarsRenderer::UpdatePeak(size_t index, float value, float deltaTime) {
        if (index >= m_peaks.size()) return;

        if (value >= m_peaks[index]) {
            m_peaks[index] = value;
            m_peakTimers[index] = PEAK_HOLD_TIME_S;
        }
        else if (m_peakTimers[index] > 0.0f) {
            m_peakTimers[index] -= deltaTime;
        }
        else {
            m_peaks[index] = std::max(0.0f, m_peaks[index] - PEAK_FALL_SPEED * deltaTime);
        }
    }

    float KenwoodBarsRenderer::GetPeakValue(size_t index) const {
        return (index < m_peaks.size()) ? m_peaks[index] : 0.0f;
    }

}