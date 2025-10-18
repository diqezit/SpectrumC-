// =-=-=-=-=-=-=-=-=-=-=
// IRenderer.h
// =-=-=-=-=-=-=-=-=-=-=

#ifndef SPECTRUM_CPP_IRENDERER_H
#define SPECTRUM_CPP_IRENDERER_H

#include "Common.h"
#include "GraphicsContext.h"

namespace Spectrum {

    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        // Main rendering function
        virtual void Render(GraphicsContext& context, const SpectrumData& spectrum) = 0;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Configuration
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        virtual void SetQuality(RenderQuality quality) = 0;
        virtual void SetPrimaryColor(const Color& color) = 0;
        virtual void SetOverlayMode(bool isOverlay) = 0;

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Information
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        virtual RenderStyle GetStyle() const = 0;
        virtual std::string_view GetName() const = 0;
        virtual bool SupportsPrimaryColor() const { return true; }

        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Lifecycle
        // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // Called when the renderer becomes active
        virtual void OnActivate(int width, int height) {}

        // Called when the renderer is no longer active
        virtual void OnDeactivate() {}
    };

}

#endif