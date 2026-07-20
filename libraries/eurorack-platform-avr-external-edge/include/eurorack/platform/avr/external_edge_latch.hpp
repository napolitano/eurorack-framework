/**
 * @file include/eurorack/platform/avr/external_edge_latch.hpp
 * @brief Declares interrupt-driven rising-edge counters for the ATmega328P's INT0 and INT1 pins.
 *
 * @details
 * Configures the external interrupt hardware to trigger on rising edges of INT0 (pin PD2) and
 * INT1 (pin PD3) and counts edges in the background, so no edge is lost to software polling
 * latency even between calls into application code. Each pin has its own saturating 8-bit
 * counter; call `consume0()`/`consume1()` periodically to atomically read and reset the count
 * since the previous call. `level0High()`/`level1High()` separately report the pin's
 * instantaneous logic level, independent of edge counting.
 *
 * This header requires an AVR target and direct access to `EICRA`/`EIFR`/`EIMSK`/`PIND` and the
 * `INT0_vect`/`INT1_vect` interrupt vectors; it is written for the ATmega328P's external-interrupt
 * register layout and its Arduino Uno/Nano pin mapping (INT0 = D2, INT1 = D3). All public members
 * are static: the class models the two fixed external-interrupt pins directly rather than an
 * instantiable object, and its state is shared between the interrupt handlers and any calling
 * code.
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

#if !defined(__AVR__)
#error "external_edge_latch.hpp requires an AVR target"
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <eurorack/compat/avr/cstdint.hpp>
#include <util/atomic.h>

namespace eurorack::platform::avr {

/**
 * @brief Interrupt-driven rising-edge counters for INT0 (D2) and INT1 (D3).
 */
class ExternalEdgeLatch final {
  public:
    /**
     * @brief Configures both INT0 and INT1 for rising-edge triggering and resets all counters.
     *
     * @details
     * Sets `EICRA` so both pins trigger on a rising edge, clears any interrupt flags already
     * pending in `EIFR` from before initialization, enables both interrupts in `EIMSK`, and
     * resets both counters and both sticky overflow flags to their initial state. The caller must
     * ensure global interrupts are enabled (`sei()`) for edges to be counted, and must install
     * `ISR(INT0_vect)`/`ISR(INT1_vect)` calling `on0()`/`on1()`, as this translation unit already
     * does.
     */
    static void initializeRisingEdges() noexcept {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            EICRA = static_cast<std::uint8_t>((EICRA & 0xF0U) | _BV(ISC01) | _BV(ISC00) |
                                              _BV(ISC11) | _BV(ISC10));
            EIFR = _BV(INTF0) | _BV(INTF1);
            EIMSK |= _BV(INT0) | _BV(INT1);
            count0_ = count1_ = 0U;
            overflow0_ = overflow1_ = false;
        }
    }

    /**
     * @brief Atomically reads and resets the INT0 rising-edge count.
     *
     * @return Number of rising edges observed on INT0 since the previous `consume0()` call (or
     * since `initializeRisingEdges()`, if this is the first call), saturating at 255; does not
     * affect `overflow0()`.
     */
    static std::uint8_t consume0() noexcept {
        std::uint8_t value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = count0_;
            count0_ = 0U;
        }
        return value;
    }

    /**
     * @brief Atomically reads and resets the INT1 rising-edge count.
     *
     * @return Number of rising edges observed on INT1 since the previous `consume1()` call (or
     * since `initializeRisingEdges()`, if this is the first call), saturating at 255; does not
     * affect `overflow1()`.
     */
    static std::uint8_t consume1() noexcept {
        std::uint8_t value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = count1_;
            count1_ = 0U;
        }
        return value;
    }

    /**
     * @brief Reads the instantaneous logic level of the INT0 pin (PD2), independent of edge
     * counting.
     *
     * @return True when PD2 currently reads high.
     */
    static bool level0High() noexcept {
        return (PIND & _BV(PD2)) != 0U;
    }

    /**
     * @brief Reads the instantaneous logic level of the INT1 pin (PD3), independent of edge
     * counting.
     *
     * @return True when PD3 currently reads high.
     */
    static bool level1High() noexcept {
        return (PIND & _BV(PD3)) != 0U;
    }

    /**
     * @brief Reports whether the INT0 counter has ever saturated since initialization.
     *
     * @details
     * This flag is sticky: it is cleared only by `initializeRisingEdges()`, not by `consume0()`,
     * so it reflects whether an overflow has ever occurred since the last (re)initialization
     * rather than since the last consumed interval.
     *
     * @return True if 255 rising edges accumulated on INT0 without an intervening `consume0()`
     * call, at any point since initialization.
     */
    static bool overflow0() noexcept {
        return overflow0_;
    }

    /**
     * @brief Reports whether the INT1 counter has ever saturated since initialization.
     *
     * @details
     * This flag is sticky: it is cleared only by `initializeRisingEdges()`, not by `consume1()`.
     *
     * @return True if 255 rising edges accumulated on INT1 without an intervening `consume1()`
     * call, at any point since initialization.
     */
    static bool overflow1() noexcept {
        return overflow1_;
    }

    /**
     * @brief Services one INT0 rising-edge event.
     *
     * @details
     * Intended to be called only from `ISR(INT0_vect)`. Increments the counter, or sets the
     * sticky overflow flag instead of wrapping once the counter reaches 255.
     */
    static void on0() noexcept {
        if (count0_ == 0xFFU) {
            overflow0_ = true;
        } else {
            ++count0_;
        }
    }

    /**
     * @brief Services one INT1 rising-edge event.
     *
     * @details
     * Intended to be called only from `ISR(INT1_vect)`. Increments the counter, or sets the
     * sticky overflow flag instead of wrapping once the counter reaches 255.
     */
    static void on1() noexcept {
        if (count1_ == 0xFFU) {
            overflow1_ = true;
        } else {
            ++count1_;
        }
    }

  private:
    static volatile std::uint8_t count0_; ///< Saturating rising-edge count for INT0 since the
                                          ///< last `consume0()` call.
    static volatile std::uint8_t count1_; ///< Saturating rising-edge count for INT1 since the
                                          ///< last `consume1()` call.
    static volatile bool overflow0_;      ///< Sticky flag set once `count0_` has saturated since
                                          ///< initialization.
    static volatile bool overflow1_;      ///< Sticky flag set once `count1_` has saturated since
                                          ///< initialization.
};

} // namespace eurorack::platform::avr
