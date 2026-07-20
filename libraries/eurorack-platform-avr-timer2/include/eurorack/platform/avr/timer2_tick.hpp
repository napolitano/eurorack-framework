/** @file timer2_tick.hpp @brief ATmega328P Timer2 CTC tick source.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */
#pragma once
#if !defined(__AVR__)
#error "timer2_tick.hpp requires an AVR target"
#endif
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <eurorack/compat/avr/cstdint.hpp>
namespace eurorack::platform::avr {
class Timer2Tick final {
 public:
  static void initialize1kHz() noexcept { ATOMIC_BLOCK(ATOMIC_RESTORESTATE){TCCR2A=_BV(WGM21);TCCR2B=_BV(CS22)|_BV(CS20);OCR2A=124U;TCNT2=0U;pending_=0U;overruns_=0U;TIMSK2|=_BV(OCIE2A);} }
  static void stop() noexcept { ATOMIC_BLOCK(ATOMIC_RESTORESTATE){TIMSK2&=static_cast<std::uint8_t>(~_BV(OCIE2A));TCCR2B=0U;} }
  static std::uint8_t consumePending() noexcept { std::uint8_t v;ATOMIC_BLOCK(ATOMIC_RESTORESTATE){v=pending_;pending_=0U;}return v; }
  static std::uint16_t overrunCount() noexcept {std::uint16_t v;ATOMIC_BLOCK(ATOMIC_RESTORESTATE){v=overruns_;}return v;}
  static void onCompareMatch() noexcept {if(pending_==0xFFU){if(overruns_!=0xFFFFU)++overruns_;}else ++pending_;}
 private: static volatile std::uint8_t pending_; static volatile std::uint16_t overruns_;
};
} // namespace eurorack::platform::avr
