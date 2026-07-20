/**
 * @file include/eurorack/platform/avr/saturating_tick_counter.hpp
 * @brief Declares hardware-independent pending-tick and overrun bookkeeping for Timer2Tick.
 *
 * @details
 * Holds exactly the algorithmic state of a periodic tick source: a saturating count of ticks
 * not yet consumed, and a saturating count of ticks lost because that counter was already
 * saturated. Nothing in this class touches a register, a timer peripheral, or any AVR-specific
 * facility, so it compiles and runs identically on a desktop build and is exercised directly by
 * native unit tests. `Timer2Tick` wraps one instance of this class and is the only part of that
 * pair that talks to real Timer2 hardware.
 *
 * @ingroup platform_avr
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif

namespace eurorack::platform::avr {

/**
 * @brief Saturating pending-tick counter with a separate saturating overrun count.
 *
 * @details
 * Not internally synchronized: a caller sharing an instance between an interrupt handler and
 * mainline code is responsible for making each individual method call atomic with respect to the
 * other context, exactly as `Timer2Tick` does with `ATOMIC_BLOCK` around every call from
 * mainline code.
 */
class SaturatingTickCounter final {
  public:
    /**
     * @brief Records one tick.
     *
     * @details
     * Increments the pending-tick count, or increments the separate overrun count instead once
     * the pending-tick count has reached 255, so a tick is never silently lost to wraparound.
     */
    void onTick() noexcept {
        if (pending_ == 0xFFU) {
            if (overruns_ != 0xFFFFU) {
                ++overruns_;
            }
        } else {
            ++pending_;
        }
    }

    /**
     * @brief Reads and resets the pending-tick count.
     *
     * @return Number of ticks recorded since the previous `consumePending()` call (or since
     * construction or `reset()`, if this is the first call), saturating at 255.
     */
    std::uint8_t consumePending() noexcept {
        const std::uint8_t value = pending_;
        pending_ = 0U;
        return value;
    }

    /**
     * @brief Reads the accumulated overrun count without resetting it.
     *
     * @details
     * Unlike `consumePending()`, reading this value does not reset it; it accumulates across
     * `consumePending()` calls and is only reset by `reset()`.
     *
     * @return Number of ticks that occurred while the pending-tick count was already saturated at
     * 255, since construction or the last `reset()` call, saturating at 65535 rather than
     * wrapping.
     */
    [[nodiscard]] std::uint16_t overrunCount() const noexcept {
        return overruns_;
    }

    /**
     * @brief Clears both the pending-tick count and the overrun count.
     */
    void reset() noexcept {
        pending_ = 0U;
        overruns_ = 0U;
    }

  private:
    std::uint8_t pending_{0U};   ///< Saturating count of ticks since the last `consumePending()`
                                 ///< call.
    std::uint16_t overruns_{0U}; ///< Saturating count of ticks lost to a saturated `pending_`
                                 ///< count since construction or the last `reset()` call.
};

} // namespace eurorack::platform::avr
