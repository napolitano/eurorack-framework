/**
 * @file src/simulation/scenario.cpp
 * @brief Implements deterministic simulation event sequences.
 *
 * @details
 * Stores and orders deterministic simulation events by virtual timestamp.
 * Events are stable-sorted so actions scheduled for the same time retain insertion order. The
 * scenario owns callable actions and labels for native tests. It does not advance time or execute
 * events itself; a runner or test loop decides when each event becomes due.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#if !defined(ARDUINO)

#if defined(__AVR__)
#include <eurorack/compat/avr/algorithm.hpp>
#else
#include <algorithm>
#endif
#include <eurorack/simulation/scenario.hpp>

namespace eurorack::simulation {

void Scenario::addDigitalEvent(const std::uint64_t atMicroseconds,
                               const std::size_t channel,
                               const bool high) {
    events_.push_back({atMicroseconds, ScenarioEventType::DigitalInput, channel, high ? 1U : 0U});
}

void Scenario::addAnalogEvent(const std::uint64_t atMicroseconds,
                              const std::size_t channel,
                              const std::uint32_t code) {
    events_.push_back({atMicroseconds, ScenarioEventType::AnalogInput, channel, code});
}

void Scenario::sort() noexcept {
    std::stable_sort(
        events_.begin(), events_.end(), [](const ScenarioEvent& left, const ScenarioEvent& right) {
            return left.atMicroseconds < right.atMicroseconds;
        });
}

const std::vector<ScenarioEvent>& Scenario::events() const noexcept {
    return events_;
}

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
