/**
 * @file include/eurorack/platform/avr/sticky_overflow_edge_counter.hpp
 * @brief Declares a hardware-independent saturating edge counter with a sticky overflow flag.
 *
 * @details
 * Holds exactly the algorithmic state of one `ExternalEdgeLatch` pin: a saturating count of
 * edges not yet consumed, and a sticky flag recording whether that count has ever saturated
 * since the last reset. Nothing in this class touches a register, an interrupt pin, or any
 * AVR-specific facility, so it compiles and runs identically on a desktop build and is exercised
 * directly by native unit tests. `ExternalEdgeLatch` wraps one instance of this class per pin and
 * is the only part of that pair that talks to real INT0/INT1 hardware.
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
 * @brief Saturating edge counter with a separate sticky overflow flag.
 *
 * @details
 * Not internally synchronized: a caller sharing an instance between an interrupt handler and
 * mainline code is responsible for making each individual method call atomic with respect to the
 * other context, exactly as `ExternalEdgeLatch` does with `ATOMIC_BLOCK` around every call from
 * mainline code.
 */
class StickyOverflowEdgeCounter final {
  public:
    /**
     * @brief Records one edge.
     *
     * @details
     * Increments the count, or sets the sticky overflow flag instead of wrapping once the count
     * has reached 255.
     */
    void onEdge() noexcept {
        if (count_ == 0xFFU) {
            overflow_ = true;
        } else {
            ++count_;
        }
    }

    /**
     * @brief Reads and resets the edge count.
     *
     * @details
     * Does not affect `overflowed()`, which is cleared only by `reset()`.
     *
     * @return Number of edges recorded since the previous `consume()` call (or since
     * construction or `reset()`, if this is the first call), saturating at 255.
     */
    std::uint8_t consume() noexcept {
        const std::uint8_t value = count_;
        count_ = 0U;
        return value;
    }

    /**
     * @brief Reports whether the count has ever saturated since construction or the last
     * `reset()` call.
     *
     * @details
     * This flag is sticky: it is cleared only by `reset()`, not by `consume()`, so it reflects
     * whether an overflow has ever occurred since the last reset rather than since the last
     * consumed interval.
     *
     * @return True if 255 edges accumulated without an intervening `consume()` call, at any
     * point since construction or the last `reset()` call.
     */
    [[nodiscard]] bool overflowed() const noexcept {
        return overflow_;
    }

    /**
     * @brief Clears both the edge count and the sticky overflow flag.
     */
    void reset() noexcept {
        count_ = 0U;
        overflow_ = false;
    }

  private:
    std::uint8_t count_{0U}; ///< Saturating edge count since the last `consume()` call.
    bool overflow_{false};   ///< Sticky flag set once `count_` has saturated since construction
                             ///< or the last `reset()` call.
};

} // namespace eurorack::platform::avr
