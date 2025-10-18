// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// GaugeRenderer.h: Renders the spectrum as a classic analog VU meter.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef SPECTRUM_CPP_GAUGE_RENDERER_H
#define SPECTRUM_CPP_GAUGE_RENDERER_H

#include "BaseRenderer.h"

namespace Spectrum {

    class GaugeRenderer final : public BaseRenderer {
    public:
        GaugeRenderer();
        ~GaugeRenderer() override = default;

        RenderStyle GetStyle() const override { return RenderStyle::Gauge; }
        std::string_view GetName() const override { return "Gauge"; }
        bool SupportsPrimaryColor() const override { return false; }

    protected:
        void UpdateSettings() override;
        void UpdateAnimation(const SpectrumData& spectrum, float deltaTime) override;
        void DoRender(GraphicsContext& context, const SpectrumData& spectrum) override;

    private:
        struct Settings {
            bool useGlow;
            bool useGradients;
            bool useHighlights;
            float smoothingFactorIncrease;
            float smoothingFactorDecrease;
        };

        void DrawGaugeBackground(GraphicsContext& context, const Rect& rect);
        void DrawVuText(GraphicsContext& context, const Rect& backgroundRect);
        void DrawScale(GraphicsContext& context, const Rect& rect);
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
        void DrawNeedle(GraphicsContext& context, const Rect& rect);
        void DrawNeedleCenter(
            GraphicsContext& context,
            const Point& center,
            float radius
        );
        void DrawPeakLamp(GraphicsContext& context, const Rect& rect);

        float DbToAngle(float db) const;
        float GetTickLength(float value, bool isMajor) const;
        float CalculateLoudness(const SpectrumData& spectrum) const;

        Settings m_settings;
        float m_currentDbValue;
        float m_currentNeedleAngle;
        int m_peakHoldCounter;
        bool m_peakActive;
    };

}

#endif