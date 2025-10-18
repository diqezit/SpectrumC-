// =-=-=-=-=-=-=-=-=-=-=
// KenwoodBarsRenderer.h
// =-=-=-=-=-=-=-=-=-=-=

#ifndef SPECTRUM_CPP_KENWOOD_BARS_RENDERER_H
#define SPECTRUM_CPP_KENWOOD_BARS_RENDERER_H

#include "BaseRenderer.h"
#include "RenderUtils.h"

namespace Spectrum {

    class KenwoodBarsRenderer final : public BaseRenderer {
    public:
        KenwoodBarsRenderer();
        ~KenwoodBarsRenderer() override = default;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // IRenderer Implementation
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        RenderStyle GetStyle() const override { return RenderStyle::KenwoodBars; }
        std::string_view GetName() const override { return "Kenwood Bars"; }
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
        // Settings & Data Structs
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        struct QualitySettings {
            bool useGradient;
            bool useRoundCorners;
            bool useOutline;
            bool useEnhancedPeaks;
        };

        struct BarData {
            Rect rect;
            float magnitude;
        };
        struct PeakData {
            Rect rect;
        };
        struct RenderData {
            std::vector<BarData> bars;
            std::vector<PeakData> peaks;
        };

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Logic Helpers
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void EnsurePeakArraySize(size_t size);
        void UpdatePeak(size_t index, float value, float deltaTime);
        float GetPeakValue(size_t index) const;

        RenderData CalculateRenderData(
            const SpectrumData& spectrum,
            const RenderUtils::BarLayout& layout
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Drawing Helpers
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void RenderMainLayer(
            GraphicsContext& context,
            const RenderData& data,
            const RenderUtils::BarLayout& layout
        );
        void RenderOutlineLayer(
            GraphicsContext& context,
            const RenderData& data,
            const RenderUtils::BarLayout& layout
        );
        void RenderPeakLayer(
            GraphicsContext& context,
            const RenderData& data,
            const RenderUtils::BarLayout& layout
        );
        void RenderPeakEnhancementLayer(
            GraphicsContext& context,
            const RenderData& data
        );

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Member State
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        QualitySettings m_currentSettings;
        std::vector<float> m_peaks;
        std::vector<float> m_peakTimers;
    };

}

#endif