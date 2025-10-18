// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// GaugeRenderer.cpp: Implementation of the GaugeRenderer class.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "GaugeRenderer.h"
#include "Utils.h"
#include "RenderUtils.h"

namespace Spectrum {

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Constants and Static Data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    namespace {
        constexpr float DB_MAX = 5.0f;
        constexpr float DB_MIN = -30.0f;
        constexpr float ANGLE_START = -150.0f;
        constexpr float ANGLE_END = -30.0f;
        constexpr float ANGLE_TOTAL_RANGE = ANGLE_END - ANGLE_START;
        constexpr int PEAK_HOLD_DURATION_FRAMES = 15;

        const std::vector<std::pair<float, const wchar_t*>> MAJOR_MARKS = {
            {-30.0f, L"-30"}, {-20.0f, L"-20"}, {-10.0f, L"-10"},
            {-7.0f,  L"-7"},  {-5.0f,  L"-5"},  {-3.0f,  L"-3"},
            {0.0f,   L"0"},   {3.0f,   L"+3"},  {5.0f,   L"+5"}
        };
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Lifecycle and Settings
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GaugeRenderer::GaugeRenderer()
        : m_currentDbValue(DB_MIN),
        m_currentNeedleAngle(ANGLE_START),
        m_peakHoldCounter(0),
        m_peakActive(false) {
        UpdateSettings();
    }

    void GaugeRenderer::UpdateSettings() {
        switch (m_quality) {
        case RenderQuality::Low:
            m_settings = { false, false, false, 0.2f, 0.05f };
            break;
        case RenderQuality::High:
            m_settings = { true, true, true, 0.15f, 0.04f };
            break;
        case RenderQuality::Medium:
        default:
            m_settings = { true, true, true, 0.2f, 0.05f };
            break;
        }
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Update Logic
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GaugeRenderer::UpdateAnimation(const SpectrumData& spectrum, float /*deltaTime*/) {
        float targetDb = CalculateLoudness(spectrum);

        float smoothingFactor = (targetDb > m_currentDbValue)
            ? m_settings.smoothingFactorIncrease
            : m_settings.smoothingFactorDecrease;

        m_currentDbValue = Utils::Lerp(m_currentDbValue, targetDb, smoothingFactor);
        float targetAngle = DbToAngle(m_currentDbValue);
        m_currentNeedleAngle = Utils::Lerp(m_currentNeedleAngle, targetAngle, 0.2f);

        if (targetDb >= 0.0f) {
            m_peakActive = true;
            m_peakHoldCounter = PEAK_HOLD_DURATION_FRAMES;
        }
        else if (m_peakHoldCounter > 0) {
            m_peakHoldCounter--;
        }
        else {
            m_peakActive = false;
        }
    }

    float GaugeRenderer::CalculateLoudness(const SpectrumData& spectrum) const {
        if (spectrum.empty()) return DB_MIN;

        float sumOfSquares = 0.0f;
        for (float val : spectrum) {
            sumOfSquares += val * val;
        }
        float rms = std::sqrt(sumOfSquares / spectrum.size());
        float db = 20.0f * std::log10(std::max(rms, 1e-10f));

        return Utils::Clamp(db, DB_MIN, DB_MAX);
    }

    float GaugeRenderer::DbToAngle(float db) const {
        float normalized = (Utils::Clamp(db, DB_MIN, DB_MAX) - DB_MIN) / (DB_MAX - DB_MIN);
        return ANGLE_START + normalized * ANGLE_TOTAL_RANGE;
    }

    float GaugeRenderer::GetTickLength(float value, bool isMajor) const {
        if (!isMajor) {
            return 0.06f; // Minor tick
        }
        if (value == 0.0f) {
            return 0.15f; // Zero dB tick
        }
        return 0.08f; // Major tick
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Render Logic
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void GaugeRenderer::DoRender(GraphicsContext& context, const SpectrumData& /*spectrum*/) {
        float aspectRatio = 2.0f;
        float viewWidth = static_cast<float>(m_width);
        float viewHeight = static_cast<float>(m_height);

        float gaugeWidth = (viewWidth / viewHeight > aspectRatio)
            ? viewHeight * 0.8f * aspectRatio
            : viewWidth * 0.8f;
        float gaugeHeight = gaugeWidth / aspectRatio;

        Rect gaugeRect = {
            (viewWidth - gaugeWidth) / 2.0f,
            (viewHeight - gaugeHeight) / 2.0f,
            gaugeWidth,
            gaugeHeight
        };

        DrawGaugeBackground(context, gaugeRect);
        DrawScale(context, gaugeRect);
        DrawNeedle(context, gaugeRect);
        DrawPeakLamp(context, gaugeRect);
    }

    void GaugeRenderer::DrawGaugeBackground(GraphicsContext& context, const Rect& rect) {
        context.DrawRoundedRectangle(rect, 8.0f, Color::FromRGB(80, 80, 80), true);

        Rect innerRect = { rect.x + 4, rect.y + 4, rect.width - 8, rect.height - 8 };
        context.DrawRoundedRectangle(innerRect, 6.0f, Color::FromRGB(105, 105, 105), true);

        Rect backgroundRect = { innerRect.x + 4, innerRect.y + 4, innerRect.width - 8, innerRect.height - 8 };
        context.DrawGradientRectangle(
            backgroundRect,
            Color::FromRGB(250, 250, 240),
            Color::FromRGB(230, 230, 215),
            false
        );
        DrawVuText(context, backgroundRect);
    }

    void GaugeRenderer::DrawVuText(GraphicsContext& context, const Rect& backgroundRect) {
        Point pos = {
            backgroundRect.x + backgroundRect.width * 0.5f - 12.0f,
            backgroundRect.y + backgroundRect.height * 0.65f
        };
        context.DrawText(L"VU", pos, Color::Black(), backgroundRect.height * 0.2f);
    }

    void GaugeRenderer::DrawScale(GraphicsContext& context, const Rect& rect) {
        Point center = { rect.x + rect.width / 2.0f, rect.y + rect.height * 0.65f };
        Point radius = { rect.width * 0.45f, rect.height * 0.5f };

        for (const auto& mark : MAJOR_MARKS) {
            DrawMark(context, center, radius, mark.first, mark.second);
        }
    }

    void GaugeRenderer::DrawMark(
        GraphicsContext& context,
        const Point& center,
        const Point& radius,
        float value,
        const wchar_t* label
    ) {
        float angle = DbToAngle(value);
        float rad = Utils::DegToRad(angle);
        float tickLength = radius.y * (value == 0.0f ? 0.15f : 0.08f);

        Point start = {
            center.x + (radius.x - tickLength) * std::cos(rad),
            center.y + (radius.y - tickLength) * std::sin(rad)
        };
        Point end = {
            center.x + radius.x * std::cos(rad),
            center.y + radius.y * std::sin(rad)
        };

        Color tickColor = (value >= 0) ? Color::Red() : Color::Black();
        context.DrawLine(start, end, tickColor, 1.8f);

        if (label) {
            DrawTickLabel(context, center, radius, value, label, angle);
        }
    }

    void GaugeRenderer::DrawTickLabel(
        GraphicsContext& context,
        const Point& center,
        const Point& radius,
        float value,
        const std::wstring& label,
        float angle
    ) {
        float rad = Utils::DegToRad(angle);
        float textOffset = radius.y * 0.12f;
        float textSize = radius.y * (value == 0.0f ? 0.11f : 0.1f);

        Point pos = {
            center.x + (radius.x + textOffset) * std::cos(rad),
            center.y + (radius.y + textOffset) * std::sin(rad)
        };

        // Adjust position based on angle to simulate text alignment
        if (angle < -120) pos.x -= 20;
        else if (angle > -60) pos.x += 5;
        else pos.x -= 10;
        pos.y -= textSize / 2.0f;

        Color textColor = (value >= 0) ? Color::Red() : Color::Black();
        context.DrawText(label, pos, textColor, textSize);
    }

    void GaugeRenderer::DrawNeedle(GraphicsContext& context, const Rect& rect) {
        Point center = { rect.x + rect.width / 2.0f, rect.y + rect.height * 0.65f };
        float radius = std::min(rect.width, rect.height) * 0.7f;
        float rad = Utils::DegToRad(m_currentNeedleAngle);
        float baseWidth = 2.5f;

        Point tip = {
            center.x + radius * std::cos(rad),
            center.y + radius * std::sin(rad)
        };
        Point baseLeft = {
            center.x - baseWidth * std::sin(rad),
            center.y + baseWidth * std::cos(rad)
        };
        Point baseRight = {
            center.x + baseWidth * std::sin(rad),
            center.y - baseWidth * std::cos(rad)
        };

        context.DrawPolygon({ tip, baseLeft, baseRight }, Color::Black(), true);
        DrawNeedleCenter(context, center, rect.width * 0.02f);
    }

    void GaugeRenderer::DrawNeedleCenter(
        GraphicsContext& context,
        const Point& center,
        float radius
    ) {
        if (m_settings.useGradients) {
            context.DrawRadialGradient(
                center,
                radius,
                Color::White(),
                Color::FromRGB(60, 60, 60)
            );
        }
        else {
            context.DrawCircle(center, radius, Color::FromRGB(60, 60, 60), true);
        }
        if (m_settings.useHighlights) {
            Point highlightCenter = { center.x - radius * 0.25f, center.y - radius * 0.25f };
            context.DrawCircle(highlightCenter, radius * 0.4f, Color(1, 1, 1, 0.6f), true);
        }
    }

    void GaugeRenderer::DrawPeakLamp(GraphicsContext& context, const Rect& rect) {
        float lampRadius = rect.height * 0.1f;
        Point lampCenter = {
            rect.x + rect.width * 0.9f,
            rect.y + rect.height * 0.2f
        };

        if (m_peakActive && m_settings.useGlow) {
            context.DrawRadialGradient(
                lampCenter,
                lampRadius * 2.5f,
                Color(1, 0, 0, 0.3f),
                Color(1, 0, 0, 0.0f)
            );
        }

        Color lampColor = m_peakActive ? Color::Red() : Color::FromRGB(80, 0, 0);
        Color centerColor = m_peakActive ? Color::White() : Color::FromRGB(180, 0, 0);
        context.DrawRadialGradient(lampCenter, lampRadius, centerColor, lampColor);

        context.DrawCircle(lampCenter, lampRadius, Color::FromRGB(40, 40, 40), false, 1.2f);

        context.DrawText(
            L"PEAK",
            { lampCenter.x - 14, lampCenter.y + lampRadius + 2 },
            lampColor,
            lampRadius
        );
    }
}