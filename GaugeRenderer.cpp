// =-=-=-=-=-=-=-=-=-=-=
// GaugeRenderer.cpp
// =-=-=-=-=-=-=-=-=-=-=

#include "GaugeRenderer.h"
#include "Utils.h"
#include <algorithm>
#include <numeric>
#include <vector>

namespace Spectrum {

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Constants and static data
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    namespace {
        // Value constants
        constexpr float DB_MAX = 5.0f;
        constexpr float DB_MIN = -30.0f;
        constexpr float DB_PEAK_THRESHOLD = 3.0f;
        constexpr float ANGLE_START = -150.0f;
        constexpr float ANGLE_END = -30.0f;
        constexpr float ANGLE_TOTAL_RANGE = ANGLE_END - ANGLE_START;
        constexpr int   PEAK_HOLD_DURATION = 15;

        // Geometry constants (standard mode)
        constexpr float BG_OUTER_CORNER_RADIUS = 8.0f;
        constexpr float BG_INNER_PADDING = 4.0f;
        constexpr float BG_INNER_CORNER_RADIUS = 6.0f;
        constexpr float BG_BACKGROUND_PADDING = 4.0f;
        constexpr float BG_VU_TEXT_SIZE_RATIO = 0.2f;
        constexpr float BG_VU_TEXT_BOTTOM_OFFSET = 0.2f;
        constexpr float NEEDLE_CENTER_Y_OFFSET = 0.4f;
        constexpr float NEEDLE_LENGTH_MULTIPLIER = 1.55f;
        constexpr float NEEDLE_BASE_WIDTH = 2.5f;
        constexpr float NEEDLE_CENTER_RADIUS = 0.02f;
        constexpr float SCALE_CENTER_Y_OFFSET = 0.15f;
        constexpr float SCALE_RADIUS_X = 0.45f;
        constexpr float SCALE_RADIUS_Y = 0.5f;
        constexpr float SCALE_TICK_LENGTH_ZERO = 0.15f;
        constexpr float SCALE_TICK_LENGTH = 0.08f;
        constexpr float SCALE_TICK_LENGTH_MINOR = 0.06f;
        constexpr float SCALE_TEXT_OFFSET = 0.12f;
        constexpr float SCALE_TEXT_SIZE_RATIO = 0.1f;
        constexpr float SCALE_TEXT_SIZE_ZERO_MULTIPLIER = 1.15f;
        constexpr float PEAK_LAMP_RADIUS = 0.05f;
        constexpr float PEAK_LAMP_X_OFFSET = 0.1f;
        constexpr float PEAK_LAMP_Y_OFFSET = 0.2f;
        constexpr float PEAK_LAMP_TEXT_Y_OFFSET = 2.5f;
        constexpr float PEAK_LAMP_GLOW_RADIUS = 1.5f;
        constexpr float PEAK_LAMP_INNER_RADIUS = 0.8f;

        // Geometry constants (overlay mode)
        constexpr float NEEDLE_CENTER_Y_OFFSET_OVERLAY = 0.35f;
        constexpr float NEEDLE_LENGTH_MULTIPLIER_OVERLAY = 1.6f;
        constexpr float NEEDLE_CENTER_RADIUS_OVERLAY = 0.015f;
        constexpr float SCALE_RADIUS_X_OVERLAY = 0.4f;
        constexpr float SCALE_RADIUS_Y_OVERLAY = 0.45f;
        constexpr float SCALE_TICK_LENGTH_ZERO_OVERLAY = 0.12f;
        constexpr float SCALE_TICK_LENGTH_OVERLAY = 0.07f;
        constexpr float SCALE_TICK_LENGTH_MINOR_OVERLAY = 0.05f;
        constexpr float SCALE_TEXT_OFFSET_OVERLAY = 0.1f;
        constexpr float SCALE_TEXT_SIZE_RATIO_OVERLAY = 0.08f;
        constexpr float PEAK_LAMP_RADIUS_OVERLAY = 0.04f;
        constexpr float PEAK_LAMP_X_OFFSET_OVERLAY = 0.12f;
        constexpr float PEAK_LAMP_Y_OFFSET_OVERLAY = 0.18f;

        // Scale marks
        const std::vector<std::pair<float, const wchar_t*>> MAJOR_MARKS = {
            {-30.0f, L"-30"}, {-20.0f, L"-20"}, {-10.0f, L"-10"},
            {-7.0f,  L"-7"},  {-5.0f,  L"-5"},  {-3.0f,  L"-3"},
            {0.0f,   L"0"},   {3.0f,   L"+3"},  {5.0f,   L"+5"}
        };

        // Fills gaps between major marks for better readability
        std::vector<float> InitializeMinorMarks() {
            std::vector<float> minorMarks;
            constexpr int MINOR_MARKS_DIVISOR = 3;
            std::vector<float> majorValues;
            for (const auto& mark : MAJOR_MARKS) majorValues.push_back(mark.first);
            std::sort(majorValues.begin(), majorValues.end());
            for (size_t i = 0; i < majorValues.size() - 1; ++i) {
                float start = majorValues[i];
                float end = majorValues[i + 1];
                float interval = end - start;
                if (interval <= 1.1f) continue;
                float step = interval / (interval > 5.0f ? 5.0f : MINOR_MARKS_DIVISOR);
                for (float val = start + step; val < end - 0.1f; val += step) {
                    minorMarks.push_back(val);
                }
            }
            return minorMarks;
        }
        const std::vector<float> MINOR_MARK_VALUES = InitializeMinorMarks();

        // Color palettes and gradient stops
        const std::vector<D2D1_GRADIENT_STOP> GAUGE_BACKGROUND_STOPS = {
            {0.0f, D2D1::ColorF(250 / 255.f, 250 / 255.f, 240 / 255.f)},
            {1.0f, D2D1::ColorF(230 / 255.f, 230 / 255.f, 215 / 255.f)}
        };
        const std::vector<D2D1_GRADIENT_STOP> NEEDLE_CENTER_STOPS = {
            {0.0f, D2D1::ColorF(D2D1::ColorF::White)},
            {0.3f, D2D1::ColorF(180 / 255.f, 180 / 255.f, 180 / 255.f)},
            {1.0f, D2D1::ColorF(60 / 255.f, 60 / 255.f, 60 / 255.f)}
        };
        const std::vector<D2D1_GRADIENT_STOP> ACTIVE_LAMP_STOPS = {
            {0.0f, D2D1::ColorF(D2D1::ColorF::White)},
            {0.3f, D2D1::ColorF(1.0f, 180 / 255.f, 180 / 255.f)},
            {1.0f, D2D1::ColorF(D2D1::ColorF::Red)}
        };
        const std::vector<D2D1_GRADIENT_STOP> INACTIVE_LAMP_STOPS = {
            {0.0f, D2D1::ColorF(220 / 255.f, 220 / 255.f, 220 / 255.f)},
            {0.3f, D2D1::ColorF(180 / 255.f, 0.f, 0.f)},
            {1.0f, D2D1::ColorF(80 / 255.f, 0.f, 0.f)}
        };
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Class Implementation
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    GaugeRenderer::GaugeRenderer()
        : m_currentDbValue(DB_MIN),
        m_currentNeedleAngle(ANGLE_START),
        m_peakHoldCounter(0),
        m_peakActive(false) {
        // This renderer requires a fixed aspect ratio
        m_aspectRatio = 2.0f;
        m_padding = 0.8f;
        UpdateSettings();
    }

    void GaugeRenderer::UpdateSettings() {
        switch (m_quality) {
        case RenderQuality::Low:
            m_currentSettings = {
                false, false, false,
                0.2f, 0.05f,
                0.15f
            };
            break;
        case RenderQuality::High:
            m_currentSettings = {
                true, true, true,
                0.15f, 0.04f,
                0.2f
            };
            break;
        case RenderQuality::Medium:
        default:
            m_currentSettings = {
                true, true, true,
                0.2f, 0.05f,
                0.15f
            };
            break;
        }
    }

    void GaugeRenderer::UpdateAnimation(
        const SpectrumData& spectrum,
        float /*deltaTime*/
    ) {
        float targetDb = CalculateLoudness(spectrum);

        float smoothingFactor = (targetDb > m_currentDbValue)
            ? m_currentSettings.smoothingFactorIncrease
            : m_currentSettings.smoothingFactorDecrease;

        if (m_isOverlay) smoothingFactor *= 0.5f;

        m_currentDbValue = Utils::Lerp(m_currentDbValue, targetDb, smoothingFactor);
        float targetAngle = DbToAngle(m_currentDbValue);
        m_currentNeedleAngle = Utils::Lerp(
            m_currentNeedleAngle, targetAngle, m_currentSettings.riseSpeed
        );

        if (targetDb >= DB_PEAK_THRESHOLD) {
            m_peakActive = true;
            m_peakHoldCounter = PEAK_HOLD_DURATION;
        }
        else if (m_peakHoldCounter > 0) {
            m_peakHoldCounter--;
        }
        else {
            m_peakActive = false;
        }
    }

    void GaugeRenderer::DoRender(
        GraphicsContext& context,
        const SpectrumData& /*spectrum*/
    ) {
        // Use the helper from BaseRenderer to get the main drawing area
        Rect gaugeRect = CalculatePaddedRect();

        if (gaugeRect.width <= 0 || gaugeRect.height <= 0) return;

        DrawGaugeBackground(context, gaugeRect);
        DrawScale(context, gaugeRect);
        DrawNeedle(context, gaugeRect);
        DrawPeakLamp(context, gaugeRect);
    }

    // Creates a layered look for the gauge casing
    void GaugeRenderer::DrawGaugeBackground(
        GraphicsContext& context,
        const Rect& rect
    ) {
        context.DrawRoundedRectangle(
            rect, BG_OUTER_CORNER_RADIUS, Color::FromRGB(80, 80, 80), true
        );

        Rect innerRect(
            rect.x + BG_INNER_PADDING,
            rect.y + BG_INNER_PADDING,
            rect.width - BG_INNER_PADDING * 2,
            rect.height - BG_INNER_PADDING * 2
        );
        context.DrawRoundedRectangle(
            innerRect, BG_INNER_CORNER_RADIUS, Color::FromRGB(105, 105, 105), true
        );

        Rect backgroundRect(
            innerRect.x + BG_BACKGROUND_PADDING,
            innerRect.y + BG_BACKGROUND_PADDING,
            innerRect.width - BG_BACKGROUND_PADDING * 2,
            innerRect.height - BG_BACKGROUND_PADDING * 2
        );
        context.DrawGradientRectangle(backgroundRect, GAUGE_BACKGROUND_STOPS, false);

        DrawVuText(context, backgroundRect, rect.height);
    }

    void GaugeRenderer::DrawVuText(
        GraphicsContext& context,
        const Rect& backgroundRect,
        float fullHeight
    ) {
        Point pos = {
            backgroundRect.x + backgroundRect.width * 0.5f,
            backgroundRect.GetBottom()
                - backgroundRect.height * BG_VU_TEXT_BOTTOM_OFFSET
        };
        context.DrawText(
            L"VU",
            pos,
            Color::Black(),
            fullHeight * BG_VU_TEXT_SIZE_RATIO,
            DWRITE_TEXT_ALIGNMENT_CENTER
        );
    }

    void GaugeRenderer::DrawScale(
        GraphicsContext& context,
        const Rect& rect
    ) {
        float centerX = rect.x + rect.width / 2.0f;
        float centerY = rect.y + rect.height / 2.0f
            + rect.height * SCALE_CENTER_Y_OFFSET;
        Point center = { centerX, centerY };

        float radiusX = rect.width
            * (m_isOverlay ? SCALE_RADIUS_X_OVERLAY : SCALE_RADIUS_X);
        float radiusY = rect.height
            * (m_isOverlay ? SCALE_RADIUS_Y_OVERLAY : SCALE_RADIUS_Y);
        Point radius = { radiusX, radiusY };

        for (const auto& mark : MAJOR_MARKS) {
            DrawMark(context, center, radius, mark.first, mark.second);
        }
        for (const auto& value : MINOR_MARK_VALUES) {
            DrawMark(context, center, radius, value, nullptr);
        }
    }

    // Renders a single tick mark and its optional label
    void GaugeRenderer::DrawMark(
        GraphicsContext& context,
        const Point& center,
        const Point& radius,
        float value,
        const wchar_t* label
    ) {
        float angle = DbToAngle(value);
        float rad = Utils::DegToRad(angle);
        float tickLength = radius.y * GetTickLength(value, label != nullptr);

        float cos_rad = std::cos(rad);
        float sin_rad = std::sin(rad);

        Point start = {
            center.x + (radius.x - tickLength) * cos_rad,
            center.y + (radius.y - tickLength) * sin_rad
        };
        Point end = {
            center.x + radius.x * cos_rad,
            center.y + radius.y * sin_rad
        };

        Color tickColor = (value >= 0)
            ? Color::FromRGB(220, 0, 0)
            : Color::FromRGB(80, 80, 80);
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
        float textOffset = radius.y
            * (m_isOverlay ? SCALE_TEXT_OFFSET_OVERLAY : SCALE_TEXT_OFFSET);
        float textSize = radius.y
            * (m_isOverlay ? SCALE_TEXT_SIZE_RATIO_OVERLAY : SCALE_TEXT_SIZE_RATIO);

        if (value == 0.0f) textSize *= SCALE_TEXT_SIZE_ZERO_MULTIPLIER;

        Point pos = {
            center.x + (radius.x + textOffset) * std::cos(rad),
            center.y + (radius.y + textOffset) * std::sin(rad)
        };

        DWRITE_TEXT_ALIGNMENT alignment = (angle < -120.0f)
            ? DWRITE_TEXT_ALIGNMENT_TRAILING
            : (angle > -60.0f)
            ? DWRITE_TEXT_ALIGNMENT_LEADING
            : DWRITE_TEXT_ALIGNMENT_CENTER;

        Color textColor = (value >= 0) ? Color::FromRGB(200, 0, 0) : Color::Black();
        context.DrawText(label, pos, textColor, textSize, alignment);
    }

    void GaugeRenderer::DrawNeedle(
        GraphicsContext& context,
        const Rect& rect
    ) {
        float centerYOffset = rect.height
            * (m_isOverlay ? NEEDLE_CENTER_Y_OFFSET_OVERLAY : NEEDLE_CENTER_Y_OFFSET);
        Point center = {
            rect.x + rect.width / 2.0f,
            rect.y + rect.height / 2.0f + centerYOffset
        };

        float radiusX = rect.width * (m_isOverlay ? SCALE_RADIUS_X_OVERLAY : SCALE_RADIUS_X);
        float radiusY = rect.height * (m_isOverlay ? SCALE_RADIUS_Y_OVERLAY : SCALE_RADIUS_Y);

        float needleLength = std::min(radiusX, radiusY)
            * (m_isOverlay ? NEEDLE_LENGTH_MULTIPLIER_OVERLAY : NEEDLE_LENGTH_MULTIPLIER);
        float centerRadius = rect.width
            * (m_isOverlay ? NEEDLE_CENTER_RADIUS_OVERLAY : NEEDLE_CENTER_RADIUS);

        DrawNeedleShape(context, center, m_currentNeedleAngle, needleLength);
        DrawNeedleCenter(context, center, centerRadius);
    }

    // Draws the needle using transformations for cleaner code
    void GaugeRenderer::DrawNeedleShape(
        GraphicsContext& context,
        const Point& center,
        float angle,
        float needleLength
    ) {
        // Defines a simple triangle shape in local space (pointing up)
        Point tip = { 0.0f, -needleLength };
        Point baseLeft = { -NEEDLE_BASE_WIDTH, 0.0f };
        Point baseRight = { NEEDLE_BASE_WIDTH, 0.0f };

        // Creates rotation and translation matrices
        D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(
            angle + 90.0f, // Add 90 because our model points up, not right
            D2D1::Point2F(0.0f, 0.0f)
        );
        D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(
            center.x, center.y
        );

        // Apply transform, draw, then reset
        context.SetTransform(rotation * translation);
        context.DrawPolygon({ tip, baseLeft, baseRight }, Color::Black(), true);
        context.ResetTransform();
    }

    // Adds a metallic-looking pivot for the needle
    void GaugeRenderer::DrawNeedleCenter(
        GraphicsContext& context,
        const Point& center,
        float radius
    ) {
        if (m_currentSettings.useGradients) {
            context.DrawRadialGradient(center, radius, NEEDLE_CENTER_STOPS);
        }
        else {
            context.DrawCircle(center, radius, Color::FromRGB(60, 60, 60), true);
        }

        if (m_currentSettings.useHighlights) {
            Point highlightCenter = { center.x - radius * 0.25f, center.y - radius * 0.25f };
            context.DrawCircle(highlightCenter, radius * 0.4f, Color(1, 1, 1, 0.6f), true);
        }
    }

    void GaugeRenderer::DrawPeakLamp(
        GraphicsContext& context,
        const Rect& rect
    ) {
        float lampRadius = std::min(rect.width, rect.height)
            * (m_isOverlay ? PEAK_LAMP_RADIUS_OVERLAY : PEAK_LAMP_RADIUS);
        Point lampCenter = {
            rect.GetRight() - rect.width * (m_isOverlay ? PEAK_LAMP_X_OFFSET_OVERLAY : PEAK_LAMP_X_OFFSET),
            rect.y + rect.height * (m_isOverlay ? PEAK_LAMP_Y_OFFSET_OVERLAY : PEAK_LAMP_Y_OFFSET)
        };

        if (m_peakActive && m_currentSettings.useGlow) {
            std::vector<D2D1_GRADIENT_STOP> glowStops = {
                {0.0f, D2D1::ColorF(1.f, 0.f, 0.f, 0.3f)},
                {1.0f, D2D1::ColorF(1.f, 0.f, 0.f, 0.0f)}
            };
            context.DrawRadialGradient(
                lampCenter,
                lampRadius * PEAK_LAMP_GLOW_RADIUS * 2.f,
                glowStops
            );
        }

        context.DrawRadialGradient(
            lampCenter,
            lampRadius * PEAK_LAMP_INNER_RADIUS,
            m_peakActive ? ACTIVE_LAMP_STOPS : INACTIVE_LAMP_STOPS
        );
        context.DrawCircle(lampCenter, lampRadius, Color::FromRGB(40, 40, 40), false, 1.2f);

        Point textPos = {
            lampCenter.x,
            lampCenter.y + lampRadius + PEAK_LAMP_TEXT_Y_OFFSET
        };
        Color textColor = m_peakActive ? Color::Red() : Color::FromRGB(180, 0, 0);

        context.DrawText(
            L"PEAK",
            textPos,
            textColor,
            lampRadius,
            DWRITE_TEXT_ALIGNMENT_CENTER
        );
    }

    float GaugeRenderer::GetTickLength(float value, bool isMajor) const {
        if (!isMajor) return m_isOverlay
            ? SCALE_TICK_LENGTH_MINOR_OVERLAY
            : SCALE_TICK_LENGTH_MINOR;

        if (value == 0.0f) return m_isOverlay
            ? SCALE_TICK_LENGTH_ZERO_OVERLAY
            : SCALE_TICK_LENGTH_ZERO;

        return m_isOverlay
            ? SCALE_TICK_LENGTH_OVERLAY
            : SCALE_TICK_LENGTH;
    }

    float GaugeRenderer::CalculateLoudness(const SpectrumData& spectrum) const {
        if (spectrum.empty()) return DB_MIN;
        float sumOfSquares = 0.0f;
        for (float val : spectrum) sumOfSquares += val * val;
        float rms = std::sqrt(sumOfSquares / spectrum.size());
        // Protect against log(0) which is -infinity
        float db = 20.0f * std::log10(std::max(rms, 1e-10f));
        return Utils::Clamp(db, DB_MIN, DB_MAX);
    }

    float GaugeRenderer::DbToAngle(float db) const {
        float normalized = (Utils::Clamp(db, DB_MIN, DB_MAX) - DB_MIN) / (DB_MAX - DB_MIN);
        return ANGLE_START + normalized * ANGLE_TOTAL_RANGE;
    }

}