/**
 * @file src/platform/avr/external_edge_latch.cpp
 * @brief Defines the INT0 and INT1 rising-edge latch storage and installs their ISRs.
 *
 * @details
 * Owns INT0 and INT1 and stores rising-edge counts in saturating, interrupt-safe latches.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/platform/avr/external_edge_latch.hpp>

namespace eurorack::platform::avr {

volatile std::uint8_t ExternalEdgeLatch::count0_ = 0U;
volatile std::uint8_t ExternalEdgeLatch::count1_ = 0U;
volatile bool ExternalEdgeLatch::overflow0_ = false;
volatile bool ExternalEdgeLatch::overflow1_ = false;

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
