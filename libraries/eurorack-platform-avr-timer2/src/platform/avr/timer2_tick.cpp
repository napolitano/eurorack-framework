/**
 * @file src/platform/avr/timer2_tick.cpp
 * @brief Defines Timer2Tick's counter storage and installs the compare-match ISR.
 *
 * @details
 * Owns the Timer2 compare-match ISR; all tick/overrun bookkeeping itself lives in
 * SaturatingTickCounter.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/platform/avr/timer2_tick.hpp>

namespace eurorack::platform::avr {

SaturatingTickCounter Timer2Tick::counter_;

} // namespace eurorack::platform::avr

// Forwards the Timer2 compare-match interrupt to Timer2Tick. ISR* is excluded from Doxygen's
// undocumented-entity check via EXCLUDE_SYMBOLS in the Doxyfile, rather than given a full brief
// block, which the project's own conventions reserve for header declarations.
ISR(TIMER2_COMPA_vect) {
    eurorack::platform::avr::Timer2Tick::onCompareMatch();
}
