/** @file external_edge_latch.cpp @brief Defines INT0 and INT1 latches.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 
 * @details Owns INT0 and INT1 and stores rising-edge counts in saturating interrupt-safe latches.*/
#include <eurorack/platform/avr/external_edge_latch.hpp>
namespace eurorack::platform::avr {volatile std::uint8_t ExternalEdgeLatch::count0_=0U;volatile std::uint8_t ExternalEdgeLatch::count1_=0U;volatile bool ExternalEdgeLatch::overflow0_=false;volatile bool ExternalEdgeLatch::overflow1_=false;}
ISR(INT0_vect){eurorack::platform::avr::ExternalEdgeLatch::on0();} ISR(INT1_vect){eurorack::platform::avr::ExternalEdgeLatch::on1();}
