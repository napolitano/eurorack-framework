/**
 * @file include/eurorack/io/time_source.hpp
 * @brief Declares monotonic time and optional delay interfaces.
 *
 * @details
 * Controls and drivers consume monotonic timestamps without depending on Arduino millis(), an RTOS,
 * or a desktop clock.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 *
 * This file is part of Eurorack Framework. Noncommercial use, modification,
 * and redistribution are permitted under the terms in the repository-root
 * LICENSE file. The separate ADDITIONAL_PERMISSION.md file permits limited
 * five-unit, cost-recovery distribution under its stated conditions.
 * Commercial use requires prior written permission.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup io
 */

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif

namespace eurorack::io {

/**
 * @brief Monotonic time source for controls, drivers, tests, and simulators.
 */
class TimeSource {
  public:
    /**
     * @brief Destroys a time-source backend through its interface.
     */
    virtual ~TimeSource() = default;

    /**
     * @brief Returns monotonic elapsed time in milliseconds.
     *
     * @return Unsigned monotonic timestamp in milliseconds.
     *
     * Natural unsigned wraparound is permitted. Consumers must use wrap-safe
     * subtraction rather than comparing absolute timestamps across wraparound.
     */
    [[nodiscard]] virtual std::uint32_t milliseconds() const noexcept = 0;

    /**
     * @brief Returns monotonic elapsed time in microseconds.
     *
     * @return Unsigned monotonic timestamp in microseconds.
     */
    [[nodiscard]] virtual std::uint32_t microseconds() const noexcept = 0;
};

/**
 * @brief Optional blocking delay service for low-level initialization.
 */
class DelayProvider {
  public:
    /**
     * @brief Destroys a delay backend through its interface.
     */
    virtual ~DelayProvider() = default;

    /**
     * @brief Blocks for at least the requested number of microseconds.
     *
     * @param microseconds Minimum requested blocking duration.
     *
     * Real-time control paths should prefer nonblocking state machines. This
     * service exists primarily for device reset and initialization sequences.
     */
    virtual void delayMicroseconds(std::uint32_t microseconds) noexcept = 0;
};

} // namespace eurorack::io
