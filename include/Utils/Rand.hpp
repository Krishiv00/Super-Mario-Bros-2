#ifndef RAND_HPP
#define RAND_HPP

#include <cstdint>

class Rand {
public:
    enum : uint8_t {
        OffsetSpawning = 1u,
        OffsetMovement = 2u
    };

private:
    static inline uint8_t m_State[7u];

public:
    static void Reset() {
        m_State[0u] = 82u;
        m_State[1u] = 128u;

        for (uint8_t i = 2u; i < 7u; ++i) {
            m_State[i] = 0u;
        }
    }

    static void Update() {
        const uint8_t bit0 = (m_State[0u] >> 1u) & 1u;
        const uint8_t bit1 = (m_State[1u] >> 1u) & 1u;
        
        uint8_t carry = bit0 ^ bit1;

        for (uint8_t i = 0u; i < 7u; i++) {
            const uint8_t old = m_State[i];
            const uint8_t newByte = (old >> 1u) | (carry << 7u);

            carry = old & 1u;
            m_State[i] = newByte;
        }
    }

    [[nodiscard]] static inline const uint8_t& RandomInt(uint8_t offset) {
        return m_State[offset];
    }
};

#endif // !RAND_HPP