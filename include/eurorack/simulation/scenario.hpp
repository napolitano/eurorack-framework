/**
 * @file include/eurorack/simulation/scenario.hpp
 * @brief Declares deterministic simulation events.
 *
 * @details
 * Schedules callback-free digital and analog input changes against virtual time.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
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
 * @brief One timestamped action in a deterministic native simulation.
 *
 * @details
 * The event owns its callable and label. Events scheduled for equal timestamps
 * retain insertion order after stable sorting.
 */
struct ScenarioEvent final {
    std::uint64_t atMicroseconds{0U};
    ScenarioEventType type{ScenarioEventType::DigitalInput};
    std::size_t channel{0U};
    std::uint32_t value{0U};
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
     * @brief Adds digital event.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param atMicroseconds Virtual event timestamp in microseconds.
     *
     * @param channel Zero-based channel index or channel identifier.
     *
     * @param high Logical state to apply.
     */
    void addDigitalEvent(std::uint64_t atMicroseconds, std::size_t channel, bool high);

    /**
     * @brief Adds analog event.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @param atMicroseconds Virtual event timestamp in microseconds.
     *
     * @param channel Zero-based channel index or channel identifier.
     *
     * @param code Raw converter code.
     */
    void addAnalogEvent(std::uint64_t atMicroseconds, std::size_t channel, std::uint32_t code);

    /**
     * @brief Orders scenario events by virtual timestamp.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     */
    void sort() noexcept;

    /**
     * @brief Returns the stored simulation events.
     *
     * @details
     * The operation is synchronous and does not retain pointers supplied only as
     * call arguments. Ownership, allocation, clipping, and error semantics follow
     * the contract documented for the enclosing type.
     *
     * @return A non-owning reference valid until the owning object is modified or destroyed.
     */
    [[nodiscard]] const std::vector<ScenarioEvent>& events() const noexcept;

  private:
    std::vector<ScenarioEvent> events_{};
};

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
