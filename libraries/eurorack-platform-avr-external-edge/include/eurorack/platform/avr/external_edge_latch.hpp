/** @file external_edge_latch.hpp @brief INT0 and INT1 rising-edge latches.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */
#pragma once
#if !defined(__AVR__)
#error "external_edge_latch.hpp requires an AVR target"
#endif
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <eurorack/compat/avr/cstdint.hpp>
namespace eurorack::platform::avr {
class ExternalEdgeLatch final { public:
 static void initializeRisingEdges() noexcept {ATOMIC_BLOCK(ATOMIC_RESTORESTATE){EICRA=static_cast<std::uint8_t>((EICRA&0xF0U)|_BV(ISC01)|_BV(ISC00)|_BV(ISC11)|_BV(ISC10));EIFR=_BV(INTF0)|_BV(INTF1);EIMSK|=_BV(INT0)|_BV(INT1);count0_=count1_=0U;overflow0_=overflow1_=false;}}
 static std::uint8_t consume0() noexcept {std::uint8_t v;ATOMIC_BLOCK(ATOMIC_RESTORESTATE){v=count0_;count0_=0U;}return v;}
 static std::uint8_t consume1() noexcept {std::uint8_t v;ATOMIC_BLOCK(ATOMIC_RESTORESTATE){v=count1_;count1_=0U;}return v;}
 static bool level0High() noexcept{return (PIND&_BV(PD2))!=0U;} static bool level1High() noexcept{return (PIND&_BV(PD3))!=0U;}
 static bool overflow0() noexcept{return overflow0_;} static bool overflow1() noexcept{return overflow1_;}
 static void on0() noexcept {if(count0_==0xFFU)overflow0_=true;else ++count0_;} static void on1() noexcept {if(count1_==0xFFU)overflow1_=true;else ++count1_;}
 private: static volatile std::uint8_t count0_,count1_; static volatile bool overflow0_,overflow1_;};
} // namespace eurorack::platform::avr
