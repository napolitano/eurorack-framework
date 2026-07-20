/**
 * @file src/platform/avr/timer2_tick.cpp
 * @brief Defines Timer2 tick storage and installs the compare-match ISR.
 *
 * @details
 * Owns the Timer2 compare-match ISR and exposes pending-tick and saturation diagnostics.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/platform/avr/timer2_tick.hpp>

namespace eurorack::platform::avr {

volatile std::uint8_t Timer2Tick::pending_ = 0U;
volatile std::uint16_t Timer2Tick::overruns_ = 0U;

} // namespace eurorack::platform::avr

// Forwards the Timer2 compare-match interrupt to Timer2Tick. Excluded from Doxygen's
// undocumented-entity check via EXCLUDE_SYMBOLS in the Doxyfile; see adc_scanner3.cpp for the
// rationale.
ISR(TIMER2_COMPA_vect) {
    eurorack::platform::avr::Timer2Tick::onCompareMatch();
}
