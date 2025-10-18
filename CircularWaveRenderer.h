// =-=-=-=-=-=-=-=-=-=-=
// CircularWaveRenderer.h
// =-=-=-=-=-=-=-=-=-=-=

#ifndef SPECTRUM_CPP_CIRCULAR_WAVE_RENDERER_H
#define SPECTRUM_CPP_CIRCULAR_WAVE_RENDERER_H

#include "BaseRenderer.h"

namespace Spectrum {

    class CircularWaveRenderer final : public BaseRenderer {
    public:
        CircularWaveRenderer();
        ~CircularWaveRenderer() override = default;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // IRenderer Implementation
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        RenderStyle GetStyle() const override { return RenderStyle::CircularWave; }
        std::string_view GetName() const override { return "Circular Wave"; }

        void OnActivate(int width, int height) override;

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
            int pointsPerCircle;
            bool useGlow;
            float maxStroke;
            int maxRings;
            float rotationSpeed;
            float waveSpeed;
            float centerRadius;
            float maxRadiusFactor;
            float minStroke;
            float waveInfluence;
            float glowThreshold;
            float glowFactor;
            float glowWidthFactor;
            float rotationIntensityFactor;
            float wavePhaseOffset;
            float strokeClampFactor;
            float minMagnitudeThreshold;
        };

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Logic & Drawing Helpers
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        void EnsureCirclePoints();
        void RenderRing(
            GraphicsContext& context,
            const SpectrumData& spectrum,
            int index,
            int totalRings,
            float ringStep,
            const Point& center,
            float maxRadius
        );
        void RenderGlowLayer(
            GraphicsContext& context,
            const Point& center,
            float radius,
            float alpha,
            float strokeWidth
        );
        void RenderMainRing(
            GraphicsContext& context,
            const Point& center,
            float radius,
            float alpha,
            float strokeWidth
        );
        void DrawCirclePath(
            GraphicsContext& context,
            const Point& center,
            float radius,
            const Color& color,
            float strokeWidth
        );

        float CalculateRingRadius(int index, float ringStep, float magnitude) const;
        float CalculateStrokeWidth(float magnitude) const;
        static float GetRingMagnitude(const SpectrumData& spectrum, int ringIndex, int ringCount);

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Member State
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        QualitySettings m_settings;
        float m_angle;
        float m_waveTime;
        std::vector<Point> m_circlePoints;
    };
}

#endif