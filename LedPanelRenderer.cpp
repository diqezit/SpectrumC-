// =-=-=-=-=-=-=-=-=-=-=
// LedPanelRenderer.cpp
// =-=-=-=-=-=-=-=-=-=-=

#include "LedPanelRenderer.h"
#include "Utils.h"

namespace Spectrum {

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Constants
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    namespace {
        constexpr float LED_RADIUS = 6.0f;
        constexpr float LED_MARGIN = 3.0f;
        constexpr float INACTIVE_ALPHA = 0.08f;
        constexpr float MIN_ACTIVE_BRIGHTNESS = 0.4f;
        constexpr float DECAY_RATE = 0.85f;
        constexpr float ATTACK_RATE = 0.4f;
        constexpr float PEAK_HOLD_TIME = 0.5f;
        constexpr float OVERLAY_PADDING_FACTOR = 0.95f;
        constexpr float PEAK_STROKE_WIDTH = 2.0f;
        constexpr float PEAK_RADIUS_OFFSET = 2.0f;
        constexpr float PEAK_DECAY_RATE = 0.95f;
        constexpr float MIN_VALUE_THRESHOLD = 0.05f;
        constexpr float TOP_LED_BRIGHTNESS_BOOST = 1.2f;
        constexpr float EXTERNAL_COLOR_BLEND = 0.7f;

        constexpr int MIN_GRID_SIZE = 10;
        constexpr int MAX_COLUMNS = 64;

        const std::vector<Color> SPECTRUM_GRADIENT = {
            Color::FromRGB(0, 200, 100), Color::FromRGB(0, 255, 0),
            Color::FromRGB(128, 255, 0), Color::FromRGB(255, 255, 0),
            Color::FromRGB(255, 200, 0), Color::FromRGB(255, 128, 0),
            Color::FromRGB(255, 64, 0), Color::FromRGB(255, 0, 0),
            Color::FromRGB(200, 0, 50)
        };
        const Color INACTIVE_COLOR = Color::FromRGB(80, 80, 80);
        const Color PEAK_COLOR = Color(1.f, 1.f, 1.f, 200.f / 255.f);
    }

    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // Class Implementation
    // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    LedPanelRenderer::LedPanelRenderer() {
        UpdateSettings();
    }

    void LedPanelRenderer::UpdateSettings() {
        if (m_isOverlay) {
            switch (m_quality) {
            case RenderQuality::Low:    m_settings = { true, 8, 1.2f }; break;
            case RenderQuality::High:   m_settings = { true, 16, 1.0f }; break;
            default:                    m_settings = { true, 12, 1.1f }; break;
            }
        }
        else {
            switch (m_quality) {
            case RenderQuality::Low:    m_settings = { false, 16, 1.0f }; break;
            case RenderQuality::High:   m_settings = { true, 32, 0.8f }; break;
            default:                    m_settings = { true, 24, 0.9f }; break;
            }
        }
        m_grid = {}; // Force grid recreation
    }

    void LedPanelRenderer::UpdateAnimation(
        const SpectrumData& spectrum,
        float deltaTime
    ) {
        if (m_grid.columns == 0) return;
        UpdateValues(spectrum);
        for (int i = 0; i < m_grid.columns; ++i) {
            UpdatePeak(i, deltaTime);
        }
    }

    void LedPanelRenderer::UpdateGridIfNeeded(size_t barCount) {
        if (m_grid.columns > 0) return;
        if (barCount == 0 || m_width <= 0 || m_height <= 0) return;

        float ledSize = LED_RADIUS * 2 + LED_MARGIN;
        float availableWidth = m_isOverlay
            ? m_width * OVERLAY_PADDING_FACTOR
            : static_cast<float>(m_width);
        float availableHeight = m_isOverlay
            ? m_height * OVERLAY_PADDING_FACTOR
            : static_cast<float>(m_height);

        int maxAllowedCols = std::min(MAX_COLUMNS, 64);
        int cols = std::min({
            maxAllowedCols,
            static_cast<int>(barCount),
            static_cast<int>(availableWidth / ledSize)
            });
        cols = std::max(MIN_GRID_SIZE, cols);

        int rows = std::min(
            m_settings.maxRows,
            static_cast<int>(availableHeight / ledSize)
        );
        rows = std::max(MIN_GRID_SIZE, rows);

        float cellSize = std::min(
            static_cast<float>(m_width) / cols,
            static_cast<float>(m_height) / rows
        );
        float gridWidth = cols * cellSize;
        float gridHeight = rows * cellSize;
        float startX = (m_width - gridWidth) * 0.5f;
        float startY = (m_height - gridHeight) * 0.5f;

        CreateGrid(cols, rows, cellSize, startX, startY);
    }

    void LedPanelRenderer::CreateGrid(
        int columns, int rows, float cellSize, float startX, float startY
    ) {
        m_grid = { rows, columns, cellSize, startX, startY };
        CacheLedPositions();
        InitializeColorGradient();
    }

    void LedPanelRenderer::CacheLedPositions() {
        m_ledPositions.assign(m_grid.columns, std::vector<Point>(m_grid.rows));
        float halfCell = m_grid.cellSize * 0.5f;
        for (int col = 0; col < m_grid.columns; ++col) {
            float x = m_grid.startX + col * m_grid.cellSize + halfCell;
            for (int row = 0; row < m_grid.rows; ++row) {
                float y = m_grid.startY
                    + (m_grid.rows - 1 - row) * m_grid.cellSize
                    + halfCell;
                m_ledPositions[col][row] = { x, y };
            }
        }
    }

    void LedPanelRenderer::InitializeColorGradient() {
        m_rowColors.resize(m_grid.rows);
        for (int i = 0; i < m_grid.rows; ++i) {
            float t = (m_grid.rows > 1)
                ? static_cast<float>(i) / (m_grid.rows - 1)
                : 0.0f;
            m_rowColors[i] = InterpolateGradient(t);
        }
    }

    Color LedPanelRenderer::InterpolateGradient(float t) {
        float scaledT = t * (SPECTRUM_GRADIENT.size() - 1);
        int index = static_cast<int>(scaledT);
        float fraction = scaledT - index;

        if (index >= static_cast<int>(SPECTRUM_GRADIENT.size()) - 1) {
            return SPECTRUM_GRADIENT.back();
        }
        return Utils::InterpolateColor(
            SPECTRUM_GRADIENT[index],
            SPECTRUM_GRADIENT[index + 1],
            fraction
        );
    }

    void LedPanelRenderer::UpdateValues(const SpectrumData& spectrum) {
        size_t count = std::min({
            static_cast<size_t>(m_grid.columns),
            spectrum.size(),
            m_smoothedValues.size()
            });
        for (size_t i = 0; i < count; ++i) {
            UpdateSmoothing(static_cast<int>(i), spectrum[i]);
        }
    }

    void LedPanelRenderer::UpdateSmoothing(int column, float target) {
        float current = m_smoothedValues[column];
        float rate = current < target ? ATTACK_RATE : (1.0f - DECAY_RATE);
        rate *= m_settings.smoothingMultiplier;
        m_smoothedValues[column] = Utils::Lerp(current, target, rate);
    }

    void LedPanelRenderer::UpdatePeak(int column, float deltaTime) {
        if (!m_settings.usePeakHold) return;

        if (m_smoothedValues[column] >= m_peakValues[column]) {
            m_peakValues[column] = m_smoothedValues[column];
            m_peakTimers[column] = PEAK_HOLD_TIME;
        }
        else if (m_peakTimers[column] > 0.0f) {
            m_peakTimers[column] -= deltaTime;
        }
        else {
            m_peakValues[column] *= PEAK_DECAY_RATE;
        }
    }

    void LedPanelRenderer::DoRender(
        GraphicsContext& context,
        const SpectrumData& spectrum
    ) {
        UpdateGridIfNeeded(spectrum.size());
        if (m_grid.columns == 0) return;

        if (m_smoothedValues.size() != m_grid.columns) {
            m_smoothedValues.assign(m_grid.columns, 0.0f);
            m_peakValues.assign(m_grid.columns, 0.0f);
            m_peakTimers.assign(m_grid.columns, 0.0f);
        }

        RenderInactiveLeds(context);
        RenderActiveLeds(context);
        if (m_settings.usePeakHold) {
            RenderPeakLeds(context);
        }
    }

    void LedPanelRenderer::RenderInactiveLeds(GraphicsContext& context) {
        Color color = INACTIVE_COLOR;
        color.a = INACTIVE_ALPHA;
        if (m_isOverlay) color.a *= OVERLAY_PADDING_FACTOR;

        for (int col = 0; col < m_grid.columns; ++col) {
            for (int row = 0; row < m_grid.rows; ++row) {
                context.DrawCircle(m_ledPositions[col][row], LED_RADIUS, color, true);
            }
        }
    }

    void LedPanelRenderer::RenderActiveLeds(GraphicsContext& context) {
        for (int col = 0; col < m_grid.columns; ++col) {
            float value = m_smoothedValues[col];
            int activeLeds = static_cast<int>(value * m_grid.rows);
            if (activeLeds == 0 && value > MIN_VALUE_THRESHOLD) {
                activeLeds = 1;
            }

            for (int row = 0; row < activeLeds; ++row) {
                bool isTopLed = (row == activeLeds - 1);
                float brightness = Utils::Lerp(MIN_ACTIVE_BRIGHTNESS, 1.0f, value);
                if (isTopLed) {
                    brightness *= TOP_LED_BRIGHTNESS_BOOST;
                }

                Color ledColor = GetLedColor(row, Utils::Saturate(brightness));
                context.DrawCircle(m_ledPositions[col][row], LED_RADIUS, ledColor, true);
            }
        }
    }

    void LedPanelRenderer::RenderPeakLeds(GraphicsContext& context) {
        for (int col = 0; col < m_grid.columns; ++col) {
            if (m_peakTimers[col] <= 0.0f) continue;

            int peakRow = static_cast<int>(m_peakValues[col] * m_grid.rows) - 1;
            if (peakRow >= 0 && peakRow < m_grid.rows) {
                context.DrawCircle(
                    m_ledPositions[col][peakRow],
                    LED_RADIUS + PEAK_RADIUS_OFFSET,
                    PEAK_COLOR,
                    false,
                    PEAK_STROKE_WIDTH
                );
            }
        }
    }


    Color LedPanelRenderer::GetLedColor(int row, float brightness) const {
        int rowIndex = std::min(
            row,
            static_cast<int>(m_rowColors.size()) - 1
        );
        Color baseColor = m_rowColors[rowIndex];

        bool useExternalColor = (m_primaryColor.r != 1.f
            || m_primaryColor.g != 1.f
            || m_primaryColor.b != 1.f);
        if (useExternalColor) {
            float t = (m_rowColors.size() > 1)
                ? static_cast<float>(row) / (m_rowColors.size() - 1)
                : 0.0f;
            baseColor = BlendWithExternalColor(baseColor, t);
        }

        baseColor.a = brightness;
        return baseColor;
    }

    Color LedPanelRenderer::BlendWithExternalColor(Color baseColor, float t) const {
        auto blend = [](float ext, float grad, float blendFactor, float t_) {
            return static_cast<float>(
                ext * blendFactor + grad * (1.0f - blendFactor) * t_
                );
            };
        return Color(
            blend(m_primaryColor.r, baseColor.r, EXTERNAL_COLOR_BLEND, t),
            blend(m_primaryColor.g, baseColor.g, EXTERNAL_COLOR_BLEND, t),
            blend(m_primaryColor.b, baseColor.b, EXTERNAL_COLOR_BLEND, t)
        );
    }

}