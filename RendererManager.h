// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RendererManager.h: Manages all available renderers and the rendering scene.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef SPECTRUM_CPP_RENDERER_MANAGER_H
#define SPECTRUM_CPP_RENDERER_MANAGER_H

#include "Common.h"
#include "IRenderer.h"
#include "GraphicsContext.h"
#include "ColorPicker.h"
#include <map>
#include <memory>

namespace Spectrum {

    class EventBus;
    class WindowManager;

    class RendererManager {
    public:
        explicit RendererManager(EventBus* bus, WindowManager* windowManager);
        ~RendererManager();

        bool Initialize();

        void RenderScene(
            GraphicsContext& graphics,
            const SpectrumData& spectrum,
            ColorPicker* colorPicker,
            bool isOverlay
        );

        void OnResize(int width, int height);

        void SetCurrentRenderer(RenderStyle style, GraphicsContext* graphics);
        void SwitchToNextRenderer(GraphicsContext* graphics);
        void CycleQuality();

        IRenderer* GetCurrentRenderer() const { return m_currentRenderer; }
        RenderStyle GetCurrentStyle() const { return m_currentStyle; }
        RenderQuality GetQuality() const { return m_currentQuality; }

    private:
        void SetQuality(RenderQuality quality);

        std::map<RenderStyle, std::unique_ptr<IRenderer>> m_renderers;
        IRenderer* m_currentRenderer = nullptr;
        RenderStyle m_currentStyle = RenderStyle::Bars;
        RenderQuality m_currentQuality = RenderQuality::Medium;
        WindowManager* m_windowManager = nullptr;
    };

}

#endif