// =-=-=-=-=-=-=-=-=-=-=
// GaugeRenderer.h
// =-=-=-=-=-=-=-=-=-=-=

#ifndef SPECTRUM_CPP_GAUGE_RENDERER_H
#define SPECTRUM_CPP_GAUGE_RENDERER_H

#include "BaseRenderer.h"

namespace Spectrum {

    class GaugeRenderer final : public BaseRenderer {
    public:
        GaugeRenderer();
        ~GaugeRenderer() override = default;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // IRenderer Implementation
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        RenderStyle GetStyle() const override { return RenderStyle::Gauge; }
        std::string_view GetName() const override { return "Gauge"; }
        bool SupportsPrimaryColor() const override { return false; }

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
        // Settings
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        struct QualitySettings {
            bool useGlow;
            bool useGradients;
            bool useHighlights;
            float smoothingFactorIncrease;
            float smoothingFactorDecrease;
            float riseSpeed;
        };

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Drawing Helpers
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void DrawGaugeBackground(
            GraphicsContext& context,
            const Rect& rect
        );

        void DrawVuText(
            GraphicsContext& context,
            const Rect& backgroundRect,
            float fullHeight
        );

        void DrawScale(
            GraphicsContext& context,
            const Rect& rect
        );

        void DrawMark(
            GraphicsContext& context,
            const Point& center,
            const Point& radius,
            float value,
            const wchar_t* label
        );

        void DrawTickLabel(
            GraphicsContext& context,
            const Point& center,
            const Point& radius,
            float value,
            const std::wstring& label,
            float angle
        );

        void DrawNeedle(
            GraphicsContext& context,
            const Rect& rect
        );

        void DrawNeedleShape(
            GraphicsContext& context,
            const Point& center,
            float angle,
            float needleLength
        );

        void DrawNeedleCenter(
            GraphicsContext& context,
            const Point& center,
            float radius
        );

        void DrawPeakLamp(
            GraphicsContext& context,
            const Rect& rect
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Calculation Helpers
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        float CalculateLoudness(const SpectrumData& spectrum) const;
        float DbToAngle(float db) const;
        float GetTickLength(float value, bool isMajor) const;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Member State
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        QualitySettings m_currentSettings;
        float m_currentDbValue;
        float m_currentNeedleAngle;
        int m_peakHoldCounter;
        bool m_peakActive;
    };

}

#endif