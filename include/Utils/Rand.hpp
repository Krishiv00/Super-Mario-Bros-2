#pragma once

#include <cstdint>

class Rand final {
public:
    enum : uint8_t {
        OffsetSpawning = 1,
        OffsetMovement = 2
    };

private:
    static inline uint8_t m_State[7];

public:
    static void Reset() {
        m_State[0] = 82;
        m_State[1] = 128;

        for (uint8_t i = 2; i < 7; ++i) {
            m_State[i] = 0;
        }
    }

    static void Update() {
        uint8_t feedback = ((m_State[0] >> 1) & 1) ^ ((m_State[1] >> 1) & 1);

        for (uint8_t& byte : m_State) {
            const uint8_t previous = byte;

            byte = (previous >> 1) | (feedback << 7);
            feedback = previous & 1;
        }
    }

    [[nodiscard]]
    static inline uint8_t RandomInt(uint8_t offset) {
        return m_State[offset];
    }
};