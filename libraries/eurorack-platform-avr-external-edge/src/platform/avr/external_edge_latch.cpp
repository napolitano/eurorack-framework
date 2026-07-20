/**
 * @file src/platform/avr/external_edge_latch.cpp
 * @brief Defines ExternalEdgeLatch's counter storage and installs the two ISRs.
 *
 * @details
 * Owns INT0 and INT1; all edge-count and overflow bookkeeping itself lives in
 * StickyOverflowEdgeCounter.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/platform/avr/external_edge_latch.hpp>

namespace eurorack::platform::avr {

StickyOverflowEdgeCounter ExternalEdgeLatch::counter0_;
StickyOverflowEdgeCounter ExternalEdgeLatch::counter1_;

} // namespace eurorack::platform::avr

// Forwards the INT0 interrupt to ExternalEdgeLatch. Excluded from Doxygen's undocumented-entity
// check via EXCLUDE_SYMBOLS in the Doxyfile; see adc_scanner3.cpp for the rationale.
ISR(INT0_vect) {
    eurorack::platform::avr::ExternalEdgeLatch::on0();
}

// Forwards the INT1 interrupt to ExternalEdgeLatch.
ISR(INT1_vect) {
    eurorack::platform::avr::ExternalEdgeLatch::on1();
}
