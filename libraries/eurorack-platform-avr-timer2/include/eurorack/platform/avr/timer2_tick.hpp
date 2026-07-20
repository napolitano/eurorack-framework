/**
 * @file include/eurorack/platform/avr/timer2_tick.hpp
 * @brief Declares a dedicated 1 kHz tick source built on the ATmega328P's Timer2 in CTC mode.
 *
 * @details
 * Configures Timer2 (not Timer0 or Timer1, which Arduino's own core already uses for `millis()`
 * and PWM) to raise a compare-match interrupt at a fixed 1 kHz, independent of any other timer
 * usage in the firmware. The interrupt only increments an in-memory tick counter; call
 * `consumePending()` periodically from the main loop to atomically read and reset the number of
 * ticks that have elapsed since the previous call, so scheduling logic can run entirely outside
 * the interrupt.
 *
 * This header requires an AVR target and direct access to `TCCR2A`/`TCCR2B`/`OCR2A`/`TCNT2`/
 * `TIMSK2` and the `TIMER2_COMPA_vect` interrupt vector; it is written for the ATmega328P's
 * Timer2 register layout and a 16 MHz system clock, not for AVR parts or clock speeds in general.
 * All public members are static: the class models the single physical Timer2 peripheral directly
 * rather than an instantiable object, and its state is shared between the interrupt handler and
 * any calling code.
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
#error "timer2_tick.hpp requires an AVR target"
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <eurorack/compat/avr/cstdint.hpp>
#include <util/atomic.h>

namespace eurorack::platform::avr {

/**
 * @brief Dedicated 1 kHz tick source using Timer2 in CTC mode, independent of Timer0/Timer1.
 */
class Timer2Tick final {
  public:
    /**
     * @brief Configures Timer2 for CTC mode at a 16 MHz system clock and starts it at 1 kHz.
     *
     * @details
     * Selects CTC mode (`TCCR2A`), a `/128` prescaler (`TCCR2B`), and a compare value of 124
     * (`OCR2A`), which together produce a compare-match interrupt every 125 timer counts at
     * 125 kHz, i.e. every 1 millisecond. Resets the counter, the pending-tick count, and the
     * overrun count, then enables the compare-match interrupt. The caller must ensure global
     * interrupts are enabled (`sei()`) for ticks to be counted, and must install
     * `ISR(TIMER2_COMPA_vect)` calling `onCompareMatch()`, as this translation unit already does.
     */
    static void initialize1kHz() noexcept {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            TCCR2A = _BV(WGM21);
            TCCR2B = _BV(CS22) | _BV(CS20);
            OCR2A = 124U;
            TCNT2 = 0U;
            pending_ = 0U;
            overruns_ = 0U;
            TIMSK2 |= _BV(OCIE2A);
        }
    }

    /**
     * @brief Disables the compare-match interrupt and stops Timer2's clock.
     *
     * @details
     * Clears `OCIE2A` in `TIMSK2` and sets `TCCR2B` to select no clock source. Does not reset the
     * pending-tick or overrun counts; call `initialize1kHz()` again to resume ticking from a
     * clean state.
     */
    static void stop() noexcept {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            TIMSK2 &= static_cast<std::uint8_t>(~_BV(OCIE2A));
            TCCR2B = 0U;
        }
    }

    /**
     * @brief Atomically reads and resets the number of ticks elapsed since the previous call.
     *
     * @return Number of 1 millisecond ticks since the previous `consumePending()` call (or since
     * `initialize1kHz()`, if this is the first call), saturating at 255 and recorded via
     * `overrunCount()` if not consumed often enough to avoid saturating.
     */
    static std::uint8_t consumePending() noexcept {
        std::uint8_t value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = pending_;
            pending_ = 0U;
        }
        return value;
    }

    /**
     * @brief Atomically reads the accumulated tick-overrun count.
     *
     * @details
     * Unlike `consumePending()`, reading this value does not reset it; it accumulates across
     * `consumePending()` calls and is only reset by `initialize1kHz()`.
     *
     * @return Number of ticks that occurred while the pending-tick counter was already saturated
     * at 255, since the last `initialize1kHz()` call, saturating at 65535 rather than wrapping.
     */
    static std::uint16_t overrunCount() noexcept {
        std::uint16_t value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = overruns_;
        }
        return value;
    }

    /**
     * @brief Services one Timer2 compare-match event.
     *
     * @details
     * Intended to be called only from `ISR(TIMER2_COMPA_vect)`. Increments the pending-tick
     * counter, or increments the saturating overrun counter instead once the pending-tick counter
     * has reached 255.
     */
    static void onCompareMatch() noexcept {
        if (pending_ == 0xFFU) {
            if (overruns_ != 0xFFFFU) {
                ++overruns_;
            }
        } else {
            ++pending_;
        }
    }

  private:
    static volatile std::uint8_t pending_;   ///< Saturating count of ticks since the last
                                             ///< `consumePending()` call.
    static volatile std::uint16_t overruns_; ///< Saturating count of ticks lost to a saturated
                                             ///< `pending_` counter since the last
                                             ///< `initialize1kHz()` call.
};

} // namespace eurorack::platform::avr
