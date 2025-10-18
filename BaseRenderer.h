// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// BaseRenderer.h: Base class for renderers providing common functionality.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef SPECTRUM_CPP_BASE_RENDERER_H
#define SPECTRUM_CPP_BASE_RENDERER_H

#include "IRenderer.h"
#include "Common.h"

namespace Spectrum {

    class BaseRenderer : public IRenderer {
    public:
        BaseRenderer();
        ~BaseRenderer() override = default;

        void SetQuality(RenderQuality quality) override;
        void SetPrimaryColor(const Color& color) override;
        void SetBackgroundColor(const Color& color) override;
        void OnActivate(int width, int height) override;

        void Render(GraphicsContext& context, const SpectrumData& spectrum) override;

    protected:
        virtual void UpdateSettings() {}
        virtual void UpdateAnimation(const SpectrumData& spectrum, float deltaTime) {}
        virtual void DoRender(GraphicsContext& context, const SpectrumData& spectrum) = 0;

        // Common state remains
        RenderQuality m_quality;
        Color m_primaryColor;
        Color m_backgroundColor;
        int m_width;
        int m_height;
        float m_time;

        bool IsRenderable(const SpectrumData& spectrum) const noexcept;

    private:
        void UpdateTime(float deltaTime);
        void SetViewport(int width, int height) noexcept;

        static constexpr float TIME_RESET_THRESHOLD = 1e6f;
    };

}

#endif