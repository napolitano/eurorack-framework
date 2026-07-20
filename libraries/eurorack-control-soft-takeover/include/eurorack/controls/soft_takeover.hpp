/**
 * @file include/eurorack/controls/soft_takeover.hpp
 * @brief Declares a generic pickup/soft-takeover policy for a physical control tracking a stored
 * target value.
 *
 * @details
 * A soft-takeover (pickup) policy solves the classic problem of a physical potentiometer or
 * fader whose position no longer matches a stored value it is meant to control, for example
 * after recalling a preset. Rather than jumping the target to the physical control's current
 * position (causing an audible or visible jump), the physical control is ignored until it either
 * comes within a configured tolerance of the target or crosses over it, at which point it takes
 * over control smoothly. The class operates on raw integer positions; it performs no I/O, holds
 * no hardware reference, and is not internally synchronized.
 *
 * @ingroup controls
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif

namespace eurorack::controls {

/**
 * @brief Tracks whether a physical control has caught up with a stored target value.
 *
 * @details
 * Call `arm()` whenever the target changes independently of the physical control (for example,
 * on preset recall), then call `update()` on every new physical sample. `update()` returns
 * whether the physical control currently owns the target; once true for a given `arm()` call, it
 * remains true until the next `arm()` call, since crossing or approaching the target is treated
 * as a one-way pickup event, not a continuous condition.
 */
class SoftTakeover final {
  public:
    /**
     * @brief Constructs a soft-takeover policy with a given pickup tolerance.
     *
     * @param tolerance Maximum absolute distance between the physical position and the target at
     * which the physical control is considered to have picked it up without crossing it.
     */
    explicit SoftTakeover(std::uint16_t tolerance = 8U) noexcept : tolerance_(tolerance) {}

    /**
     * @brief Records a new target and the physical control's position at the moment it changed.
     *
     * @details
     * Resets the pickup state to inactive: subsequent `update()` calls will ignore the physical
     * control until it approaches or crosses the new target.
     *
     * @param target New target value the physical control should eventually own.
     * @param physical Physical control's position at the moment the target changed.
     */
    void arm(std::uint16_t target, std::uint16_t physical) noexcept {
        target_ = target;
        previous_ = physical;
        active_ = false;
    }

    /**
     * @brief Updates the physical position and reports whether control now owns the target.
     *
     * @details
     * Once active, a call always returns true regardless of the new physical position; pickup is
     * a one-way transition that only `arm()` resets. While inactive, becomes active when
     * `physical` is within `tolerance` of `target`, or when the physical position has moved from
     * one side of `target` to the other since the previous call.
     *
     * @param physical Latest physical control position.
     * @return True once the physical control has picked up the target; false while still armed
     * and waiting.
     */
    bool update(std::uint16_t physical) noexcept {
        if (!active_) {
            const bool near = physical > target_ ? physical - target_ <= tolerance_
                                                 : target_ - physical <= tolerance_;
            const bool crossed = (previous_ < target_ && physical >= target_) ||
                                 (previous_ > target_ && physical <= target_);
            active_ = near || crossed;
        }

        previous_ = physical;
        return active_;
    }

    /**
     * @brief Reports whether the physical control currently owns the target.
     *
     * @return True once pickup has occurred since the last `arm()` call.
     */
    [[nodiscard]] bool active() const noexcept {
        return active_;
    }

    /**
     * @brief Returns the target value set by the most recent `arm()` call.
     *
     * @return Current target value.
     */
    [[nodiscard]] std::uint16_t target() const noexcept {
        return target_;
    }

  private:
    std::uint16_t tolerance_{8U}; ///< Maximum pickup distance configured at construction.
    std::uint16_t target_{0U};    ///< Target value set by the most recent `arm()` call.
    std::uint16_t previous_{0U};  ///< Physical position observed on the previous `update()` call.
    bool active_{false};          ///< True once the physical control has picked up the target.
};

} // namespace eurorack::controls
