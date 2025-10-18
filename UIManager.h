// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UIManager.h: Manages all UI components and their interactions.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef SPECTRUM_CPP_UI_MANAGER_H
#define SPECTRUM_CPP_UI_MANAGER_H

#include "Common.h"
#include "ColorPicker.h"
#include <memory>
#include <vector>

namespace Spectrum {

    class GraphicsContext;
    class ControllerCore;

    class UIManager {
    public:
        explicit UIManager(ControllerCore* controller);

        bool Initialize(GraphicsContext& context);
        void RecreateResources(GraphicsContext& context);

        void Draw(GraphicsContext& context);
        bool HandleMouseMessage(UINT msg, int x, int y);

        ColorPicker* GetColorPicker() const { return m_colorPicker.get(); }

    private:
        ControllerCore* m_controller; // For callbacks
        std::unique_ptr<ColorPicker> m_colorPicker;
        // Future UI elements will be added here
    };

}

#endif