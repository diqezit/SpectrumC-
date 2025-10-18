// =-=-=-=-=-=-=-=-=-=-=
// LedPanelRenderer.h
// =-=-=-=-=-=-=-=-=-=-=

#ifndef SPECTRUM_CPP_LED_PANEL_RENDERER_H
#define SPECTRUM_CPP_LED_PANEL_RENDERER_H

#include "BaseRenderer.h"

namespace Spectrum {

    class LedPanelRenderer final : public BaseRenderer {
    public:
        LedPanelRenderer();
        ~LedPanelRenderer() override = default;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // IRenderer Implementation
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        RenderStyle GetStyle() const override { return RenderStyle::LedPanel; }
        std::string_view GetName() const override { return "LED Panel"; }
        bool SupportsPrimaryColor() const override { return true; }

    protected:
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // BaseRenderer Overrides
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void UpdateSettings() override;
        void UpdateAnimation(
            const SpectrumData& spectrum,
            float deltaTime
        ) override;
        void DoRender(
            GraphicsContext& context,
            const SpectrumData& spectrum
        ) override;

    private:
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Settings & Data Structs
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        struct QualitySettings {
            bool usePeakHold;
            int maxRows;
            float smoothingMultiplier;
        };
        struct GridData {
            int rows;
            int columns;
            float cellSize;
            float startX;
            float startY;
        };

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Logic Helpers
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void UpdateGridIfNeeded(size_t barCount);
        void CreateGrid(
            int columns,
            int rows,
            float cellSize,
            float startX,
            float startY
        );
        void CacheLedPositions();
        void InitializeColorGradient();

        void UpdateValues(const SpectrumData& spectrum);
        void UpdateSmoothing(int column, float target);
        void UpdatePeak(int column, float deltaTime);

        Color GetLedColor(int row, float brightness) const;
        static Color InterpolateGradient(float t);
        Color BlendWithExternalColor(Color baseColor, float t) const;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Drawing Helpers
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void RenderInactiveLeds(GraphicsContext& context);
        void RenderActiveLeds(GraphicsContext& context);
        void RenderPeakLeds(GraphicsContext& context);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Member State
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        QualitySettings m_settings;
        GridData m_grid;

        std::vector<float> m_smoothedValues;
        std::vector<float> m_peakValues;
        std::vector<float> m_peakTimers;

        std::vector<std::vector<Point>> m_ledPositions;
        std::vector<Color> m_rowColors;
    };
}

#endif