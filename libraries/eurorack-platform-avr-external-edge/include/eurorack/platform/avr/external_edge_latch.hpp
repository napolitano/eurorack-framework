/**
 * @file include/eurorack/platform/avr/external_edge_latch.hpp
 * @brief Declares interrupt-driven rising-edge counters for the ATmega328P's INT0 and INT1 pins.
 *
 * @details
 * Configures the external interrupt hardware to trigger on rising edges of INT0 (pin PD2) and
 * INT1 (pin PD3) and counts edges in the background, so no edge is lost to software polling
 * latency even between calls into application code. Each pin has its own
 * `StickyOverflowEdgeCounter`; call `consume0()`/`consume1()` periodically to atomically read
 * and reset the count since the previous call. `level0High()`/`level1High()` separately report
 * the pin's instantaneous logic level, independent of edge counting. All algorithmic bookkeeping
 * lives in `StickyOverflowEdgeCounter`, which has no AVR dependency and is covered directly by
 * native unit tests; this class is only the thin, hardware-facing adapter that configures the
 * external-interrupt registers and the two ISRs.
 *
 * This header requires an AVR target and direct access to `EICRA`/`EIFR`/`EIMSK`/`PIND` and the
 * `INT0_vect`/`INT1_vect` interrupt vectors; it is written for the ATmega328P's external-interrupt
 * register layout and its Arduino Uno/Nano pin mapping (INT0 = D2, INT1 = D3). All public members
 * are static: the class models the two fixed external-interrupt pins directly rather than an
 * instantiable object, and its state is shared between the interrupt handlers and any calling
 * code.
 *
 * Lifecycle: before `initializeRisingEdges()` runs, INT0/INT1 are whatever the platform startup
 * code left them as (typically disabled) and no edges are counted. After
 * `initializeRisingEdges()` returns, both pins are configured for rising-edge triggering with
 * freshly reset counters, but edges are only actually recorded once global interrupts are
 * enabled (`sei()`) and `ISR(INT0_vect)`/`ISR(INT1_vect)` are reachable, both of which remain the
 * caller's responsibility. `stop()` disables both interrupts without resetting the counters;
 * call `initializeRisingEdges()` again for a full reset back to the post-initialization state.
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
#include <eurorack/platform/avr/sticky_overflow_edge_counter.hpp>
#include <util/atomic.h>

namespace eurorack::platform::avr {

/**
 * @brief Interrupt-driven rising-edge counters for INT0 (D2) and INT1 (D3).
 */
class ExternalEdgeLatch final {
  public:
    /**
     * @brief Configures both INT0 and INT1 for rising-edge triggering and resets both counters.
     *
     * @details
     * Sets `EICRA` so both pins trigger on a rising edge, clears any interrupt flags already
     * pending in `EIFR` from before initialization, enables both interrupts in `EIMSK`, and
     * resets both `StickyOverflowEdgeCounter` instances. The caller must ensure global interrupts
     * are enabled (`sei()`) for edges to be counted, and must install `ISR(INT0_vect)`/
     * `ISR(INT1_vect)` calling `on0()`/`on1()`, as this translation unit already does.
     */
    static void initializeRisingEdges() noexcept {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            EICRA = static_cast<std::uint8_t>((EICRA & 0xF0U) | _BV(ISC01) | _BV(ISC00) |
                                              _BV(ISC11) | _BV(ISC10));
            EIFR = _BV(INTF0) | _BV(INTF1);
            EIMSK |= _BV(INT0) | _BV(INT1);
            counter0_.reset();
            counter1_.reset();
        }
    }

    /**
     * @brief Disables both INT0 and INT1 interrupts.
     *
     * @details
     * Clears `INT0` and `INT1` in `EIMSK`. Does not reset either counter; call
     * `initializeRisingEdges()` again to resume edge counting from a clean state.
     */
    static void stop() noexcept {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            EIMSK &= static_cast<std::uint8_t>(~(_BV(INT0) | _BV(INT1)));
        }
    }

    /**
     * @brief Atomically reads and resets the INT0 rising-edge count.
     *
     * @return See `StickyOverflowEdgeCounter::consume`.
     */
    static std::uint8_t consume0() noexcept {
        std::uint8_t value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = counter0_.consume();
        }
        return value;
    }

    /**
     * @brief Atomically reads and resets the INT1 rising-edge count.
     *
     * @return See `StickyOverflowEdgeCounter::consume`.
     */
    static std::uint8_t consume1() noexcept {
        std::uint8_t value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = counter1_.consume();
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
     * @return See `StickyOverflowEdgeCounter::overflowed`.
     */
    static bool overflow0() noexcept {
        return counter0_.overflowed();
    }

    /**
     * @brief Reports whether the INT1 counter has ever saturated since initialization.
     *
     * @return See `StickyOverflowEdgeCounter::overflowed`.
     */
    static bool overflow1() noexcept {
        return counter1_.overflowed();
    }

    /**
     * @brief Services one INT0 rising-edge event.
     *
     * @details
     * Intended to be called only from `ISR(INT0_vect)`, which already runs with global
     * interrupts disabled, so this forwards to `StickyOverflowEdgeCounter::onEdge` without its
     * own atomic section.
     */
    static void on0() noexcept {
        counter0_.onEdge();
    }

    /**
     * @brief Services one INT1 rising-edge event.
     *
     * @details
     * Intended to be called only from `ISR(INT1_vect)`, which already runs with global
     * interrupts disabled, so this forwards to `StickyOverflowEdgeCounter::onEdge` without its
     * own atomic section.
     */
    static void on1() noexcept {
        counter1_.onEdge();
    }

  private:
    static StickyOverflowEdgeCounter counter0_; ///< Hardware-independent bookkeeping for INT0.
    static StickyOverflowEdgeCounter counter1_; ///< Hardware-independent bookkeeping for INT1.
};

} // namespace eurorack::platform::avr
