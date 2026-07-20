/**
 * @file src/platform/avr/adc_scanner3.cpp
 * @brief Implements the ADC5/ADC6/ADC7 free-running scanner.
 *
 * @details
 * Owns the ADC conversion-complete ISR and rotates ADC5, ADC6, and ADC7 with median-of-three
 * filtering, discarding the first post-switch sample on each channel to allow the sample-and-hold
 * capacitor to settle. The `/128` prescaler keeps the ADC clock within its specified maximum for
 * a 16 MHz system clock. `ready` only becomes true after every channel's three-sample window has
 * been filled at least once, so a caller reading the snapshot immediately after `initialize()`
 * can tell that the medians are not yet meaningful.

 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */

#include <eurorack/platform/avr/adc_scanner3.hpp>

namespace eurorack::platform::avr {

volatile std::uint16_t AdcScanner3::samples_[3][3] = {{0U}};
volatile std::uint8_t AdcScanner3::channel_ = 0U;
volatile std::uint8_t AdcScanner3::sampleIndex_ = 0U;
volatile std::uint8_t AdcScanner3::discard_ = 1U;
volatile bool AdcScanner3::ready_ = false;

void AdcScanner3::initialize(AdcReference reference) noexcept {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        channel_ = 0U;
        sampleIndex_ = 0U;
        discard_ = 1U;
        ready_ = false;
        ADMUX = static_cast<std::uint8_t>((reference == AdcReference::Avcc ? _BV(REFS0) : 0U) | 5U);
        ADCSRA = _BV(ADEN) | _BV(ADATE) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
        ADCSRB = 0U;
        ADCSRA |= _BV(ADSC);
    }
}

std::uint16_t AdcScanner3::median(std::uint16_t a, std::uint16_t b, std::uint16_t c) noexcept {
    if (a > b) {
        const std::uint16_t t = a;
        a = b;
        b = t;
    }
    if (b > c) {
        const std::uint16_t t = b;
        b = c;
        c = t;
    }
    if (a > b) {
        const std::uint16_t t = a;
        a = b;
        b = t;
    }
    return b;
}

AdcScanner3Snapshot AdcScanner3::snapshot() noexcept {
    AdcScanner3Snapshot result;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        result.a5 = median(samples_[0][0], samples_[0][1], samples_[0][2]);
        result.a6 = median(samples_[1][0], samples_[1][1], samples_[1][2]);
        result.a7 = median(samples_[2][0], samples_[2][1], samples_[2][2]);
        result.ready = ready_;
    }
    return result;
}

void AdcScanner3::onConversionComplete() noexcept {
    const std::uint16_t value = ADC;

    if (discard_ > 0U) {
        discard_ = 0U;
        return;
    }

    samples_[channel_][sampleIndex_] = value;
    ++sampleIndex_;

    if (sampleIndex_ >= 3U) {
        sampleIndex_ = 0U;
        ++channel_;
        if (channel_ >= 3U) {
            channel_ = 0U;
            ready_ = true;
        }
        ADMUX =
            static_cast<std::uint8_t>((ADMUX & 0xF0U) | static_cast<std::uint8_t>(5U + channel_));
        discard_ = 1U;
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
