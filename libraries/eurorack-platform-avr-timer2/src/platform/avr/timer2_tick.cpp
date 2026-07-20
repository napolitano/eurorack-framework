/** @file timer2_tick.cpp @brief Defines Timer2 tick storage and ISR.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 
 * @details Owns the Timer2 compare-match ISR and exposes pending ticks and saturation diagnostics.*/
#include <eurorack/platform/avr/timer2_tick.hpp>
namespace eurorack::platform::avr { volatile std::uint8_t Timer2Tick::pending_=0U; volatile std::uint16_t Timer2Tick::overruns_=0U; }
ISR(TIMER2_COMPA_vect){eurorack::platform::avr::Timer2Tick::onCompareMatch();}
