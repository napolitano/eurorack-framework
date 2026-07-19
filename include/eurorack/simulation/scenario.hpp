/**
 * @file include/eurorack/simulation/scenario.hpp
 * @brief Declares deterministic simulation events.
 *
 * @details
 * Schedules callback-free digital and analog input changes against virtual time.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup simulation
 */

#if !defined(ARDUINO)

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace eurorack::simulation {

/**
 * @brief Semantic category attached to a scheduled simulation event.
 *
 * @details
 * Event type supports readable traces and filtering; execution remains defined
 * by the event's callable action.
 */
enum class ScenarioEventType : std::uint8_t { DigitalInput, AnalogInput };

/**
 * @brief One timestamped digital or analog input change in a deterministic native simulation.
 *
 * @details
 * The event is a plain value type; it carries no callable action. A test or simulator frontend
 * interprets `type`/`channel`/`value` and applies the change itself when the event becomes due.
 * Events scheduled for equal timestamps retain insertion order after `Scenario::sort`.
 */
struct ScenarioEvent final {
    std::uint64_t atMicroseconds{0U}; ///< Virtual event timestamp in microseconds.
    ScenarioEventType type{ScenarioEventType::DigitalInput}; ///< Which channel kind `channel`
                                                             ///< refers to and how `value`
                                                             ///< should be interpreted.
    std::size_t channel{0U}; ///< Zero-based index of the target digital or analog input channel.
    std::uint32_t value{0U}; ///< For `DigitalInput`, `1` for a high level or `0` for low. For
                             ///< `AnalogInput`, the raw converter code to apply.
};

/**
 * @brief Ordered container of deterministic simulation events.
 *
 * @details
 * Scenario stores and sorts events but deliberately does not own a clock or run
 * loop. Tests or simulator frontends decide when due actions are executed.
 * Dynamic allocation is acceptable because this component is native-only.
 */
class Scenario final {
  public:
    /**
     * @brief Schedules a digital input change.
     *
     * @details
     * Appends a `DigitalInput` event; `high` is stored as `1` or `0` in `ScenarioEvent::value`.
     * Events are not sorted automatically; call `sort` before relying on timestamp order.
     *
     * @param atMicroseconds Virtual event timestamp in microseconds.
     * @param channel Zero-based digital input channel index.
     * @param high Logical level to apply when the event becomes due.
     */
    void addDigitalEvent(std::uint64_t atMicroseconds, std::size_t channel, bool high);

    /**
     * @brief Schedules an analog input change.
     *
     * @details
     * Appends an `AnalogInput` event carrying `code` in `ScenarioEvent::value`. Events are not
     * sorted automatically; call `sort` before relying on timestamp order.
     *
     * @param atMicroseconds Virtual event timestamp in microseconds.
     * @param channel Zero-based analog input channel index.
     * @param code Raw converter code to apply when the event becomes due.
     */
    void addAnalogEvent(std::uint64_t atMicroseconds, std::size_t channel, std::uint32_t code);

    /**
     * @brief Stable-sorts scheduled events by `atMicroseconds`.
     *
     * @details
     * Events with equal timestamps retain their relative insertion order.
     */
    void sort() noexcept;

    /**
     * @brief Returns the scheduled events in their current order.
     *
     * @return Constant reference to the event list, in insertion order until `sort` is called.
     */
    [[nodiscard]] const std::vector<ScenarioEvent>& events() const noexcept;

  private:
    std::vector<ScenarioEvent> events_{}; ///< Scheduled events, in insertion order until `sort`
                                          ///< is called.
};

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
