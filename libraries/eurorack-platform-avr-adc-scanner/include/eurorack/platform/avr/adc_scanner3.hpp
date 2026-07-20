/**
 * @file include/eurorack/platform/avr/adc_scanner3.hpp
 * @brief Declares an interrupt-driven, free-running scanner for ADC channels 5 through 7 on the
 * classic AVR (ATmega328P-family) ADC peripheral.
 *
 * @details
 * Configures the ADC for free-running conversion across three fixed channels (ADC5, ADC6, ADC7)
 * and services every conversion from the `ADC_vect` interrupt, so the main program never blocks
 * on a conversion. Each channel keeps a rolling window of its three most recent samples;
 * `snapshot()` returns the median of that window per channel, rejecting single-sample noise
 * spikes without averaging (which would otherwise blur a genuine step change). After each channel
 * switch, the first conversion is discarded before samples are recorded, because the sample-and-
 * hold capacitor has not yet settled to the newly selected channel's voltage.
 *
 * This header requires an AVR target and direct access to `ADMUX`/`ADCSRA`/`ADCSRB`/`ADC` and the
 * `ADC_vect` interrupt vector; it is written for the ATmega328P's ADC register layout and channel
 * numbering, not for AVR parts in general. All public members are static: the class models a
 * single, whole-device ADC peripheral rather than an instantiable object, and its state is shared
 * between the interrupt handler and any calling code.
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
                          ///< completed since `initialize()`; the medians are not yet meaningful
                          ///< while false.
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
     * Resets all scan and sample state, selects `reference` and the first channel (ADC5) in
     * `ADMUX`, enables the ADC with auto-triggered free-running conversion and the
     * conversion-complete interrupt in `ADCSRA` at a fixed prescaler, clears `ADCSRB` (free-
     * running trigger source), and starts the first conversion. The caller must ensure global
     * interrupts are enabled (`sei()`) for the scan to make progress, and must install
     * `ISR(ADC_vect)` calling `onConversionComplete()`, as this translation unit already does.
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
     * Intended to be called only from `ISR(ADC_vect)`. Discards the first conversion after every
     * channel switch, then records subsequent conversions into the current channel's three-slot
     * rolling window. Once a channel's window fills, advances to the next channel (wrapping after
     * ADC7 back to ADC5, at which point `ready` becomes true), reprograms `ADMUX` for the new
     * channel, and arms the next-switch discard.
     */
    static void onConversionComplete() noexcept;

  private:
    /**
     * @brief Returns the median of three values.
     *
     * @param a First value.
     * @param b Second value.
     * @param c Third value.
     * @return The middle value once `a`, `b`, and `c` are considered in sorted order.
     */
    static std::uint16_t median(std::uint16_t a, std::uint16_t b, std::uint16_t c) noexcept;

    static volatile std::uint16_t samples_[3][3]; ///< Rolling three-sample window per channel,
                                                  ///< indexed `[channel][sampleIndex]`.
    static volatile std::uint8_t channel_;        ///< Currently scanned channel index (0 = ADC5,
                                                  ///< 1 = ADC6, 2 = ADC7).
    static volatile std::uint8_t sampleIndex_;    ///< Next slot to fill within the current
                                                  ///< channel's rolling window.
    static volatile std::uint8_t discard_; ///< Nonzero while the next conversion for the current
                                           ///< channel must be discarded rather than recorded.
    static volatile bool ready_; ///< True once every channel has filled its rolling window at
                                 ///< least once.
};

} // namespace eurorack::platform::avr
