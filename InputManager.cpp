// InputManager.cpp
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputManager.cpp: Implementation of the InputManager.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "InputManager.h"

namespace Spectrum {

    InputManager::InputManager() {}

    void InputManager::Update() {
        PollKeys();
    }

    std::vector<InputAction> InputManager::GetActions() {
        if (m_actionQueue.empty()) {
            return {};
        }
        std::vector<InputAction> actions = std::move(m_actionQueue);
        m_actionQueue.clear();
        return actions;
    }

    void InputManager::PollKeys() {
        const std::vector<int> keysToPoll = {
            VK_SPACE, 'A', 'R', 'Q', 'O', 'S',
            VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT,
            VK_SUBTRACT, VK_OEM_MINUS,
            VK_ADD, VK_OEM_PLUS,
            VK_ESCAPE
        };

        for (int key : keysToPoll) {
            bool isPressed = (GetAsyncKeyState(key) & 0x8000) != 0;

            if (isPressed && !m_keyStates[key]) {
                m_keyStates[key] = true;
                QueueAction(key);
            }
            else if (!isPressed) {
                m_keyStates[key] = false;
            }
        }
    }

    void InputManager::QueueAction(int key) {
        switch (key) {
        case VK_SPACE:
            m_actionQueue.push_back(InputAction::ToggleCapture);
            break;
        case 'A':
            m_actionQueue.push_back(InputAction::ToggleAnimation);
            break;
        case 'S':
            m_actionQueue.push_back(InputAction::CycleSpectrumScale);
            break;
        case VK_UP:
            m_actionQueue.push_back(InputAction::IncreaseAmplification);
            break;
        case VK_DOWN:
            m_actionQueue.push_back(InputAction::DecreaseAmplification);
            break;
        case VK_LEFT:
            m_actionQueue.push_back(InputAction::PrevFFTWindow);
            break;
        case VK_RIGHT:
            m_actionQueue.push_back(InputAction::NextFFTWindow);
            break;
        case VK_SUBTRACT:
        case VK_OEM_MINUS:
            m_actionQueue.push_back(InputAction::DecreaseBarCount);
            break;
        case VK_ADD:
        case VK_OEM_PLUS:
            m_actionQueue.push_back(InputAction::IncreaseBarCount);
            break;
        case 'R':
            m_actionQueue.push_back(InputAction::SwitchRenderer);
            break;
        case 'Q':
            m_actionQueue.push_back(InputAction::CycleQuality);
            break;
        case 'O':
            m_actionQueue.push_back(InputAction::ToggleOverlay);
            break;
        case VK_ESCAPE:
            m_actionQueue.push_back(InputAction::Exit);
            break;
        default:
            break;
        }
    }

} // namespace Spectrum