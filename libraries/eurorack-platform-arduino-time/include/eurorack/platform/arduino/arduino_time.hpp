/**
 * @file include/eurorack/platform/arduino/arduino_time.hpp
 * @brief Declares Arduino time and delay adapters.
 *
 * @details
 * Wraps millis, micros, and delayMicroseconds behind framework interfaces.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup platform_arduino
 */

#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/io/time_source.hpp>

namespace eurorack::platform::arduino {

/**
 * @brief Arduino-backed monotonic time and delay provider.
 */
class ArduinoTime final : public eurorack::io::TimeSource, public eurorack::io::DelayProvider {
  public:
    /**
     * @brief Returns Arduino millis().
     *
     * @return Monotonic milliseconds with natural wraparound.
     */
    [[nodiscard]] std::uint32_t milliseconds() const noexcept override {
        return static_cast<std::uint32_t>(millis());
    }

    /**
     * @brief Returns Arduino micros().
     *
     * @return Monotonic microseconds with natural wraparound.
     */
    [[nodiscard]] std::uint32_t microseconds() const noexcept override {
        return static_cast<std::uint32_t>(micros());
    }

    /**
     * @brief Performs a blocking Arduino microsecond delay.
     *
     * @param microseconds Minimum delay duration.
     */
    void delayMicroseconds(const std::uint32_t microseconds) noexcept override {
        ::delayMicroseconds(static_cast<unsigned int>(microseconds));
    }
};

} // namespace eurorack::platform::arduino

#endif // ARDUINO
