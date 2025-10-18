// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// RendererManager.cpp: Implementation of the RendererManager class.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "RendererManager.h"
#include "BarsRenderer.h"
#include "WaveRenderer.h"
#include "CircularWaveRenderer.h"
#include "CubesRenderer.h"
#include "FireRenderer.h"
#include "LedPanelRenderer.h"
#include "GaugeRenderer.h"
#include "KenwoodBarsRenderer.h"


#include "Utils.h"
#include "EventBus.h"
#include "WindowManager.h"

namespace Spectrum {

    // RendererManager holds window manager to get graphics context for activation
    RendererManager::RendererManager(EventBus* bus, WindowManager* windowManager)
        : m_windowManager(windowManager)
    {
        bus->Subscribe(InputAction::SwitchRenderer, [this]() {
            if (m_windowManager) {
                this->SwitchToNextRenderer(m_windowManager->GetGraphics());
            }
            });
        bus->Subscribe(InputAction::CycleQuality, [this]() {
            this->CycleQuality();
            });
    }

    RendererManager::~RendererManager() {}

    bool RendererManager::Initialize() {
        m_renderers[RenderStyle::Bars] = std::make_unique<BarsRenderer>();
        m_renderers[RenderStyle::Wave] = std::make_unique<WaveRenderer>();
        m_renderers[RenderStyle::CircularWave] = std::make_unique<CircularWaveRenderer>();
        m_renderers[RenderStyle::Cubes] = std::make_unique<CubesRenderer>();
        m_renderers[RenderStyle::Fire] = std::make_unique<FireRenderer>();
        m_renderers[RenderStyle::LedPanel] = std::make_unique<LedPanelRenderer>();
        m_renderers[RenderStyle::Gauge] = std::make_unique<GaugeRenderer>();
        m_renderers[RenderStyle::KenwoodBars] = std::make_unique<KenwoodBarsRenderer>();

        m_currentStyle = RenderStyle::Bars;
        m_currentRenderer = m_renderers[m_currentStyle].get();
        SetQuality(m_currentQuality);
        return true;
    }

    void RendererManager::RenderScene(
        GraphicsContext& graphics,
        const SpectrumData& spectrum,
        ColorPicker* colorPicker,
        bool isOverlay
    ) {
        if (auto rt = graphics.GetRenderTarget()) {
            if (rt->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED) {
                return;
            }
        }

        graphics.BeginDraw();

        const Color clearColor = isOverlay
            ? Color::Transparent()
            : Color::FromRGB(13, 13, 26);
        graphics.Clear(clearColor);

        if (m_currentRenderer) {
            m_currentRenderer->Render(graphics, spectrum);
        }

        if (colorPicker && colorPicker->IsVisible() && !isOverlay) {
            colorPicker->Draw(graphics);
        }
    }

    void RendererManager::SetCurrentRenderer(
        RenderStyle style, GraphicsContext* graphics
    ) {
        if (m_currentRenderer) {
            m_currentRenderer->OnDeactivate();
        }

        m_currentRenderer = m_renderers[style].get();
        m_currentStyle = style;

        if (m_currentRenderer && graphics) {
            int w = graphics->GetWidth();
            int h = graphics->GetHeight();
            m_currentRenderer->OnActivate(w, h);
            LOG_INFO(
                "Switched to " << m_currentRenderer->GetName().data()
                << " renderer"
            );
        }
    }

    void RendererManager::SwitchToNextRenderer(GraphicsContext* graphics) {
        RenderStyle nextStyle = Utils::CycleEnum(m_currentStyle, 1);
        SetCurrentRenderer(nextStyle, graphics);
    }

    void RendererManager::SetQuality(RenderQuality quality) {
        m_currentQuality = quality;
        for (auto& [style, renderer] : m_renderers) {
            if (renderer) {
                renderer->SetQuality(quality);
            }
        }

        const char* qualityName = "Unknown";
        switch (quality) {
        case RenderQuality::Low:    qualityName = "Low"; break;
        case RenderQuality::Medium: qualityName = "Medium"; break;
        case RenderQuality::High:   qualityName = "High"; break;
        default: break;
        }
        LOG_INFO("Render quality set to " << qualityName);
    }

    void RendererManager::CycleQuality() {
        RenderQuality nextQuality = Utils::CycleEnum(m_currentQuality, 1);
        SetQuality(nextQuality);
    }

    void RendererManager::OnResize(int width, int height) {
        if (m_currentRenderer) {
            m_currentRenderer->OnActivate(width, height);
        }
    }

}