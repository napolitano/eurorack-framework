/**
 * @file include/eurorack/platform/avr/timer2_tick.hpp
 * @brief Declares a dedicated 1 kHz tick source built on the ATmega328P's Timer2 in CTC mode.
 *
 * @details
 * Configures Timer2 (not Timer0 or Timer1, which Arduino's own core already uses for `millis()`
 * and PWM) to raise a compare-match interrupt at a fixed 1 kHz, independent of any other timer
 * usage in the firmware. The interrupt only forwards to a `SaturatingTickCounter`; call
 * `consumePending()` periodically from the main loop to atomically read and reset the number of
 * ticks that have elapsed since the previous call, so scheduling logic can run entirely outside
 * the interrupt. All algorithmic bookkeeping lives in `SaturatingTickCounter`, which has no AVR
 * dependency and is covered directly by native unit tests; this class is only the thin,
 * hardware-facing adapter that configures Timer2's registers and the ISR forwarding.
 *
 * This header requires an AVR target and direct access to `TCCR2A`/`TCCR2B`/`OCR2A`/`TCNT2`/
 * `TIMSK2` and the `TIMER2_COMPA_vect` interrupt vector. The 125 kHz timer clock and 124-count
 * compare value it derives are correct only for a 16 MHz system clock; `initialize1kHz()`
 * enforces this with a `static_assert` on `F_CPU` rather than silently producing wrong timing on
 * a different clock speed. All public members are static: the class models the single physical
 * Timer2 peripheral directly rather than an instantiable object, and its state is shared between
 * the interrupt handler and any calling code.
 *
 * Lifecycle: before `initialize1kHz()` runs, Timer2's clock is whatever the platform startup
 * code left it as (typically stopped) and no ticks are counted. After `initialize1kHz()`
 * returns, Timer2 is running at 1 kHz with a freshly reset counter, but ticks are only actually
 * recorded once global interrupts are enabled (`sei()`) and `ISR(TIMER2_COMPA_vect)` is
 * reachable, both of which remain the caller's responsibility. `stop()` halts the timer and
 * disables its interrupt without resetting the counter; call `initialize1kHz()` again for a full
 * reset back to the post-initialization state.
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

#if !defined(F_CPU)
#error "timer2_tick.hpp requires F_CPU to be defined by the build (e.g. -DF_CPU=16000000UL)"
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <eurorack/compat/avr/cstdint.hpp>
#include <eurorack/platform/avr/saturating_tick_counter.hpp>
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
     * 125 kHz, i.e. every 1 millisecond; a `static_assert` rejects the build at compile time if
     * `F_CPU` is not exactly 16 MHz, since these register values would otherwise produce wrong
     * timing silently. Resets the tick counter, then enables the compare-match interrupt. The
     * caller must ensure global interrupts are enabled (`sei()`) for ticks to be counted, and
     * must install `ISR(TIMER2_COMPA_vect)` calling `onCompareMatch()`, as this translation unit
     * already does.
     */
    static void initialize1kHz() noexcept {
        static_assert(F_CPU == 16000000UL,
                      "Timer2Tick::initialize1kHz's prescaler and compare value are derived for "
                      "a 16 MHz system clock; a different F_CPU would silently change the tick "
                      "rate. Adjust TCCR2B/OCR2A for the actual clock before using this on "
                      "another F_CPU.");
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            TCCR2A = _BV(WGM21);
            TCCR2B = _BV(CS22) | _BV(CS20);
            OCR2A = 124U;
            TCNT2 = 0U;
            counter_.reset();
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
     * `initialize1kHz()`, if this is the first call); see `SaturatingTickCounter::consumePending`
     * for the saturation behavior.
     */
    static std::uint8_t consumePending() noexcept {
        std::uint8_t value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = counter_.consumePending();
        }
        return value;
    }

    /**
     * @brief Atomically reads the accumulated tick-overrun count.
     *
     * @return See `SaturatingTickCounter::overrunCount`.
     */
    static std::uint16_t overrunCount() noexcept {
        std::uint16_t value;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            value = counter_.overrunCount();
        }
        return value;
    }

    /**
     * @brief Services one Timer2 compare-match event.
     *
     * @details
     * Intended to be called only from `ISR(TIMER2_COMPA_vect)`, which already runs with global
     * interrupts disabled, so this forwards to `SaturatingTickCounter::onTick` without its own
     * atomic section.
     */
    static void onCompareMatch() noexcept {
        counter_.onTick();
    }

  private:
    static SaturatingTickCounter counter_; ///< Hardware-independent tick and overrun bookkeeping.
};

} // namespace eurorack::platform::avr
