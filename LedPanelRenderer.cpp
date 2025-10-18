// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// LedPanelRenderer.cpp: Implementation of the LedPanelRenderer class.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include "LedPanelRenderer.h"
#include "Utils.h"

namespace Spectrum {

    LedPanelRenderer::LedPanelRenderer() {
        CreateGradient();
        UpdateSettings();
    }

    void LedPanelRenderer::UpdateSettings() {
        switch (m_quality) {
        case RenderQuality::Low:
            m_settings = { 16, false, 0.0f, 0.4f };
            break;
        case RenderQuality::Medium:
            m_settings = { 24, true, 1.0f, 0.4f };
            break;
        case RenderQuality::High:
            m_settings = { 32, true, 1.5f, 0.45f };
            break;
        default:
            m_settings = { 24, true, 1.0f, 0.4f };
            break;
        }
    }

    void LedPanelRenderer::CreateGradient() {
        m_gradient = {
            Color::Green(),
            Color::FromRGB(255, 255, 0),
            Color::Red()
        };
    }

    void LedPanelRenderer::UpdateGrid(size_t barCount) {
        if (barCount <= 0 || m_width <= 0 || m_height <= 0) {
            m_grid = {};
            return;
        }

        m_grid.cols = static_cast<int>(barCount);
        m_grid.rows = m_settings.rows;
        m_grid.cellSize = std::min(
            static_cast<float>(m_width) / m_grid.cols,
            static_cast<float>(m_height) / m_grid.rows
        );

        const float gridW = m_grid.cols * m_grid.cellSize;
        const float gridH = m_grid.rows * m_grid.cellSize;
        m_grid.startX = (m_width - gridW) * 0.5f;
        m_grid.startY = (m_height - gridH) * 0.5f;
    }

    void LedPanelRenderer::UpdateAnimation(
        const SpectrumData& spectrum,
        float deltaTime
    ) {
        if (m_currentValues.size() != spectrum.size()) {
            m_currentValues.assign(spectrum.size(), 0.0f);
            m_peakValues.assign(spectrum.size(), 0.0f);
            m_peakTimers.assign(spectrum.size(), 0.0f);
        }

        const float peakDecay = 0.95f;

        for (size_t i = 0; i < spectrum.size(); ++i) {
            m_currentValues[i] = Utils::Lerp(m_currentValues[i], spectrum[i], 0.3f);

            if (m_settings.usePeakHold) {
                if (m_currentValues[i] >= m_peakValues[i]) {
                    m_peakValues[i] = m_currentValues[i];
                    m_peakTimers[i] = m_settings.peakHoldTime;
                }
                else {
                    if (m_peakTimers[i] > 0.0f) {
                        m_peakTimers[i] -= deltaTime;
                        if (m_peakTimers[i] < 0.0f) {
                            m_peakTimers[i] = 0.0f;
                        }
                    }
                    else {
                        m_peakValues[i] *= peakDecay;
                    }
                }
            }
        }
    }

    Color LedPanelRenderer::GetLedColor(
        int row,
        int totalRows,
        float brightness
    ) const {
        const float ratio = static_cast<float>(row) / totalRows;
        Color c;

        if (ratio < 0.6f) {
            const float t = ratio / 0.6f;
            c = Utils::InterpolateColor(m_gradient[0], m_gradient[1], t);
        }
        else {
            const float t = (ratio - 0.6f) / 0.4f;
            c = Utils::InterpolateColor(m_gradient[1], m_gradient[2], t);
        }

        return Utils::AdjustBrightness(c, brightness);
    }

    void LedPanelRenderer::DoRender(
        GraphicsContext& context,
        const SpectrumData& spectrum
    ) {
        UpdateGrid(spectrum.size());
        if (m_grid.rows == 0 || m_grid.cols == 0) return;

        const float ledRadius = m_grid.cellSize * m_settings.ledRadiusRatio;
        const Color offColor(0.2f, 0.2f, 0.2f, 0.2f);

        for (int y = 0; y < m_grid.rows; ++y) {
            for (int x = 0; x < m_grid.cols; ++x) {
                const Point center = {
                    m_grid.startX + x * m_grid.cellSize + m_grid.cellSize * 0.5f,
                    m_grid.startY + y * m_grid.cellSize + m_grid.cellSize * 0.5f
                };

                const int currentRow = m_grid.rows - 1 - y;
                const int litLeds = static_cast<int>(
                    Utils::Clamp(m_currentValues[x], 0.0f, 1.0f) * m_grid.rows
                    );
                const int peakLed = static_cast<int>(
                    Utils::Clamp(m_peakValues[x], 0.0f, 1.0f) * m_grid.rows
                    );

                Color ledColor = offColor;
                if (currentRow < litLeds) {
                    ledColor = GetLedColor(currentRow, m_grid.rows, 1.0f);
                }
                else if (m_settings.usePeakHold && currentRow == peakLed && m_peakTimers[x] > 0.0f) {
                    ledColor = Color::White();
                    ledColor.a = 0.8f;
                }

                context.DrawCircle(center, ledRadius, ledColor, true);
            }
        }
    }

}