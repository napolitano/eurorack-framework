/**
 * @file include/eurorack/controls/on_off_momentary_switch.hpp
 * @brief Declares a debounced On-Off-(On) three-position switch.
 *
 * @details
 * Models one maintained On contact, a center Off position, and one spring-return momentary On
 * contact using two independent electrical inputs.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup controls
 */

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/controls/momentary_button.hpp>

namespace eurorack::controls {

/** @brief Stable logical position of an On-Off-(On) switch. */
enum class OnOffMomentaryPosition : std::uint8_t {
    On,          ///< Maintained On position.
    Off,         ///< Center Off position.
    MomentaryOn, ///< Spring-return momentary On position.
    Invalid      ///< Both contacts are simultaneously asserted.
};

/** @brief Configuration for a three-position On-Off-(On) switch. */
struct OnOffMomentarySwitchConfig final {
    ActiveLevel maintainedActiveLevel{ActiveLevel::Low}; ///< Active level of maintained contact.
    ActiveLevel momentaryActiveLevel{ActiveLevel::Low};  ///< Active level of momentary contact.
    std::uint32_t debounceTimeMs{10U};                   ///< Stable time required per contact.
};

/** @brief Immutable three-position switch state. */
struct OnOffMomentarySwitchSnapshot final {
    OnOffMomentaryPosition position{OnOffMomentaryPosition::Off}; ///< Current stable position.
    OnOffMomentaryPosition previousPosition{
        OnOffMomentaryPosition::Off};  ///< Position before last accepted change.
    bool changed{false};               ///< True only for the update that changes position.
    bool invalidCombination{false};    ///< True while both contacts are asserted.
    std::uint32_t transitionCount{0U}; ///< Accepted logical position changes.
};

/**
 * @brief Debounces an On-Off-(On) switch with two contacts.
 *
 * @details
 * The center position is inferred when neither contact is asserted. Both
 * contacts asserted is reported as Invalid because that normally indicates
 * wiring trouble, contact overlap, or an impossible simulated state.
 */
class OnOffMomentarySwitch final {
  public:
    /** @brief Constructs the switch model. @param config Contact polarities and debounce. */
    explicit OnOffMomentarySwitch(OnOffMomentarySwitchConfig config = {}) noexcept;

    /**
     * @brief Resets from two electrical contact levels.
     * @param maintainedRawHigh Maintained-contact electrical level.
     * @param momentaryRawHigh Momentary-contact electrical level.
     * @param nowMs Current monotonic milliseconds.
     */
    void reset(bool maintainedRawHigh, bool momentaryRawHigh, std::uint32_t nowMs) noexcept;

    /**
     * @brief Processes one sample of both contacts.
     * @param maintainedRawHigh Maintained-contact electrical level.
     * @param momentaryRawHigh Momentary-contact electrical level.
     * @param nowMs Current monotonic milliseconds.
     */
    void update(bool maintainedRawHigh, bool momentaryRawHigh, std::uint32_t nowMs) noexcept;

    /** @brief Returns the latest state. @return Constant snapshot reference. */
    [[nodiscard]] const OnOffMomentarySwitchSnapshot& snapshot() const noexcept;

    /** @brief Returns current logical position. @return Stable position. */
    [[nodiscard]] OnOffMomentaryPosition position() const noexcept;

  private:
    /** @brief Derives logical position from debounced contacts. @return Derived position. */
    [[nodiscard]] OnOffMomentaryPosition derivePosition() const noexcept;

    OnOffMomentarySwitchConfig config_{};     ///< Contact polarities and debounce time.
    MomentaryButton maintained_{};            ///< Debounced maintained-contact submodel.
    MomentaryButton momentary_{};             ///< Debounced momentary-contact submodel.
    OnOffMomentarySwitchSnapshot snapshot_{}; ///< Most recently derived stable position.
};

} // namespace eurorack::controls
