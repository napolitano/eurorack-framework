/**
 * @file src/platform/avr/adc_scanner3.cpp
 * @brief Implements the ADC5/ADC6/ADC7 free-running scanner adapter.
 *
 * @details
 * Owns the ADC conversion-complete ISR and the ADC register configuration; all round-robin,
 * rolling-window, and median-of-three bookkeeping itself lives in MedianOfThreeScanner. The
 * `/128` prescaler keeps the ADC clock within its specified maximum for a 16 MHz system clock.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/platform/avr/adc_scanner3.hpp>

namespace eurorack::platform::avr {

MedianOfThreeScanner AdcScanner3::scanner_;

void AdcScanner3::initialize(AdcReference reference) noexcept {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        scanner_.reset();
        ADMUX = static_cast<std::uint8_t>((reference == AdcReference::Avcc ? _BV(REFS0) : 0U) | 5U);
        ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
        ADCSRB = 0U;
        ADCSRA |= _BV(ADSC);
    }
}

AdcScanner3Snapshot AdcScanner3::snapshot() noexcept {
    MedianOfThreeSnapshot model;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        model = scanner_.snapshot();
    }

    AdcScanner3Snapshot result;
    result.a5 = model.channel[0];
    result.a6 = model.channel[1];
    result.a7 = model.channel[2];
    result.ready = model.ready;
    return result;
}

void AdcScanner3::onConversionComplete() noexcept {
    const MedianOfThreeChannelSwitch event = scanner_.recordSample(ADC);
    if (event.changed) {
        ADMUX = static_cast<std::uint8_t>((ADMUX & 0xF0U) |
                                          static_cast<std::uint8_t>(5U + event.channel));
    }
}

} // namespace eurorack::platform::avr

// Forwards the ADC conversion-complete interrupt to AdcScanner3. This vector-table entry point
// is excluded from Doxygen's undocumented-entity check via EXCLUDE_SYMBOLS in the Doxyfile,
// rather than given a full brief block, which the project's own conventions reserve for header
// declarations.
ISR(ADC_vect) {
    eurorack::platform::avr::AdcScanner3::onConversionComplete();
}
