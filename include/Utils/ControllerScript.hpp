#ifndef CONTROLLER_SCRIPT_PLAYER_HPP
#define CONTROLLER_SCRIPT_PLAYER_HPP

#include <vector>

#include "Sprite/Player.hpp"

struct DemoInput {
    uint8_t inputBits = 0x00u;
    uint8_t duration = 0x00u;
};

namespace ButtonInput {
    enum ButtonInput : uint8_t {
        RIGHT = 0b00000001u,
        LEFT = 0b00000010u,
        UP = 0b00001000u,
        DOWN = 0b00000100u,
        SPRINT = 0b01000000u,
        JUMP = 0b10000000u
    };
}

class ScriptPlayer {
private:
    uint8_t m_CurrentIndex;
    uint8_t m_FrameTimer;

    std::vector<DemoInput> m_Script;

public:
    // start the demo
    void Start(std::vector<DemoInput>& script) {
        m_Script = std::move(script);

        player.m_TasMode = true;
        
        m_CurrentIndex = 0u;
        m_FrameTimer = m_Script[m_CurrentIndex].duration;

        Done = false;
    }

    void Update() {
        if (Done || m_Script.empty()) {
            return;
        }

        const DemoInput& currentInput = m_Script.at(m_CurrentIndex);

        if (currentInput.inputBits & ButtonInput::SPRINT) {
            player.m_SprintKeyHeld = Player::SprintBufferLength;
        }

        player.m_LeftKeyHeld = currentInput.inputBits & ButtonInput::LEFT;
        player.m_RightKeyHeld = currentInput.inputBits & ButtonInput::RIGHT;
        player.m_UpKeyHeld = currentInput.inputBits & ButtonInput::UP;
        player.m_DownKeyHeld = currentInput.inputBits & ButtonInput::DOWN;
        player.m_JumpKeyHeld = currentInput.inputBits & ButtonInput::JUMP;

        if (--m_FrameTimer == 0u) {
            ++m_CurrentIndex;

            if (m_CurrentIndex == m_Script.size()) {
                Done = true;
                player.m_TasMode = false;
            } else {
                m_FrameTimer = m_Script[m_CurrentIndex].duration;
            }
        }
    }

    void Stop() {
        m_Script.clear();
        player.resetInputs();
        player.m_TasMode = false;
    }

    bool Done = false;
};

class ScriptRecorder {
private:
    std::vector<DemoInput> m_Script;

    bool m_Recording = false;

    uint8_t m_InitialLevel;

public:
    void StartRecording(const uint8_t& currentLevel) {
        m_Script.clear();

        m_Recording = true;

        m_InitialLevel = currentLevel;
    }

    void StopRecording() {
        m_Recording = false;
    }

    void Reset() {
        m_Recording = false;
        m_Script.clear();
    }

    void Update() {
        if (!m_Recording) {
            return;
        }

        const bool sprintKeyHeld = player.m_SprintKeyHeld == Player::SprintBufferLength - 1;

        const uint8_t input = (
            (ButtonInput::SPRINT * sprintKeyHeld) |
            (ButtonInput::JUMP * player.m_JumpKeyHeld) |
            (ButtonInput::LEFT * player.m_LeftKeyHeld) |
            (ButtonInput::RIGHT * player.m_RightKeyHeld) |
            (ButtonInput::UP * player.m_UpKeyHeld) |
            (ButtonInput::DOWN * player.m_DownKeyHeld)
        );

        if (m_Script.empty() || m_Script.back().inputBits != input || m_Script.back().duration == 255U) {
            m_Script.emplace_back(input, 0x01u);
        } else {
            ++m_Script.back().duration;
        }
    }

    [[nodiscard]] inline const bool& Recording() const noexcept {
        return m_Recording;
    }

    [[nodiscard]] inline const std::vector<DemoInput>& getScript() const {
        return m_Script;
    }

    [[nodiscard]] inline const uint8_t& GetInitialLevel() const noexcept {
        return m_InitialLevel;
    }
};

#endif // !CONTROLLER_SCRIPT_PLAYER_HPP