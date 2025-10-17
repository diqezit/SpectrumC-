// InputManager.h
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// InputManager.h: Manages keyboard input and generates actions.
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef SPECTRUM_CPP_INPUT_MANAGER_H
#define SPECTRUM_CPP_INPUT_MANAGER_H

#include "Common.h"
#include <map>
#include <vector>

namespace Spectrum {

    class InputManager {
    public:
        InputManager();

        void Update();

        std::vector<InputAction> GetActions();

    private:
        void PollKeys();
        void QueueAction(int key);

        std::map<int, bool> m_keyStates;
        std::vector<InputAction> m_actionQueue;
    };

} // namespace Spectrum

#endif // SPECTRUM_CPP_INPUT_MANAGER_H