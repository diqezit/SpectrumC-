// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// UIManager.cpp: Implementation of the UIManager class.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "UIManager.h"
#include "GraphicsContext.h"
#include "ControllerCore.h"

namespace Spectrum {

    UIManager::UIManager(ControllerCore* controller)
        : m_controller(controller) {
    }

    bool UIManager::Initialize(GraphicsContext& context) {
        m_colorPicker = std::make_unique<ColorPicker>(Point{ 20.0f, 20.0f }, 40.0f);
        if (!m_colorPicker->Initialize(context)) {
            return false;
        }

        // Set up callback to notify the controller of color changes
        m_colorPicker->SetOnColorSelectedCallback([this](const Color& color) {
            if (m_controller) {
                m_controller->SetPrimaryColor(color);
            }
            });

        return true;
    }

    void UIManager::RecreateResources(GraphicsContext& context) {
        if (m_colorPicker) {
            m_colorPicker->RecreateResources(context);
        }
    }

    void UIManager::Draw(GraphicsContext& context) {
        if (m_colorPicker && m_colorPicker->IsVisible()) {
            m_colorPicker->Draw(context);
        }
        // Future elements will be drawn here
    }

    bool UIManager::HandleMouseMessage(UINT msg, int x, int y) {
        bool needsRedraw = false;
        if (m_colorPicker && m_colorPicker->IsVisible()) {
            if (msg == WM_MOUSEMOVE) {
                needsRedraw = m_colorPicker->HandleMouseMove(x, y);
            }
            else if (msg == WM_LBUTTONDOWN) {
                needsRedraw = m_colorPicker->HandleMouseClick(x, y);
            }
        }
        // Future elements will handle messages here

        return needsRedraw;
    }

}