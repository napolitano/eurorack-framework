/**
 * @file include/eurorack/platform/avr/adc_scanner3.hpp
 * @brief Declares an interrupt-driven, free-running scanner for ADC channels 5 through 7 on the
 * classic AVR (ATmega328P-family) ADC peripheral.
 *
 * @details
 * Configures the ADC for free-running conversion across three fixed channels (ADC5, ADC6, ADC7)
 * and services every conversion from the `ADC_vect` interrupt, so the main program never blocks
 * on a conversion. All round-robin, rolling-window, and median-of-three bookkeeping lives in
 * `MedianOfThreeScanner`, which has no AVR dependency and is covered directly by native unit
 * tests; this class is only the thin, hardware-facing adapter that configures the ADC's
 * registers and reprograms `ADMUX` whenever the model reports that it just advanced to a new
 * channel.
 *
 * This header requires an AVR target and direct access to `ADMUX`/`ADCSRA`/`ADCSRB`/`ADC` and the
 * `ADC_vect` interrupt vector; it is written for the ATmega328P's ADC register layout and channel
 * numbering, not for AVR parts in general. All public members are static: the class models a
 * single, whole-device ADC peripheral rather than an instantiable object, and its state is shared
 * between the interrupt handler and any calling code.
 *
 * Lifecycle: before `initialize()` runs, the ADC is disabled (or whatever state the platform
 * startup code left it in) and no samples are scanned. After `initialize()` returns, the ADC is
 * enabled in free-running mode with a freshly reset scan model, but conversions only actually
 * complete once global interrupts are enabled (`sei()`) and `ISR(ADC_vect)` is reachable, both of
 * which remain the caller's responsibility. There is no `stop()`: disabling the ADC mid-scan has
 * no use case in the driving quantizer application, so it was not added speculatively.
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
#error "adc_scanner3.hpp requires an AVR target"
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <eurorack/compat/avr/cstdint.hpp>
#include <eurorack/platform/avr/median_of_three_scanner.hpp>
#include <util/atomic.h>

namespace eurorack::platform::avr {

/**
 * @brief Selects the ADC voltage reference used by `AdcScanner3::initialize`.
 */
enum class AdcReference : std::uint8_t {
    ExternalAref, ///< External reference applied to the AREF pin.
    Avcc          ///< Internal AVCC reference.
};

/**
 * @brief Median-filtered snapshot of the three scanned ADC channels.
 */
struct AdcScanner3Snapshot final {
    std::uint16_t a5{0U}; ///< Median-of-three raw code for ADC5.
    std::uint16_t a6{0U}; ///< Median-of-three raw code for ADC6.
    std::uint16_t a7{0U}; ///< Median-of-three raw code for ADC7.
    bool ready{false};    ///< True once at least one full rotation through all three channels has
                          ///< completed since `initialize()`; the medians are not yet
                          ///< meaningful while false.
};

/**
 * @brief Interrupt-driven free-running scanner for ADC channels 5, 6, and 7.
 */
class AdcScanner3 final {
  public:
    /**
     * @brief Configures and starts free-running conversion across ADC5, ADC6, and ADC7.
     *
     * @details
     * Resets the scan model, selects `reference` and the first channel (ADC5) in `ADMUX`,
     * enables the ADC with auto-triggered free-running conversion and the conversion-complete
     * interrupt in `ADCSRA` at a fixed prescaler, clears `ADCSRB` (free-running trigger source),
     * and starts the first conversion. The caller must ensure global interrupts are enabled
     * (`sei()`) for the scan to make progress, and must install `ISR(ADC_vect)` calling
     * `onConversionComplete()`, as this translation unit already does.
     *
     * @param reference Voltage reference applied to `ADMUX`.
     */
    static void initialize(AdcReference reference = AdcReference::Avcc) noexcept;

    /**
     * @brief Atomically reads the current median-filtered snapshot.
     *
     * @return Snapshot with the median of the last three samples for each channel, and whether a
     * full rotation has completed at least once.
     */
    static AdcScanner3Snapshot snapshot() noexcept;

    /**
     * @brief Services one ADC conversion-complete event.
     *
     * @details
     * Intended to be called only from `ISR(ADC_vect)`, which already runs with global interrupts
     * disabled. Forwards the raw `ADC` reading to `MedianOfThreeScanner::recordSample`, and
     * reprograms `ADMUX` for the newly active channel whenever that call reports a channel
     * switch.
     */
    static void onConversionComplete() noexcept;

  private:
    static MedianOfThreeScanner scanner_; ///< Hardware-independent round-robin scan bookkeeping.
};

} // namespace eurorack::platform::avr
