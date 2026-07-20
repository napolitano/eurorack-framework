/**
 * @file include/eurorack/controls/press_classifier.hpp
 * @brief Declares a timestamp-driven button press-gesture classifier.
 *
 * @details
 * Classifies an already-debounced pressed/released boolean stream into short press, double
 * press, long press, and auto-repeat gestures, emitting at most one gesture per `update()` call.
 * A short press is reported only after its double-press window has elapsed without a second
 * click, so short and double presses are always mutually exclusive from the caller's point of
 * view. The class holds only timestamps and small flags; it performs no I/O and is not
 * internally synchronized.
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
 * @brief Gesture reported by `PressClassifier`, at most one per `update()`/`poll()` call.
 */
enum class PressEvent : std::uint8_t {
    None,        ///< No gesture completed on this call.
    ShortPress,  ///< A single press-and-release with no second click within `doublePressMs`.
    DoublePress, ///< A second press-and-release completed within `doublePressMs` of the first.
    LongPress,   ///< The button has been held continuously for at least `longPressMs`.
    Repeat       ///< A further auto-repeat tick while still held past the long-press point.
};

/**
 * @brief Timing thresholds for gesture classification.
 */
struct PressClassifierConfig final {
    std::uint32_t longPressMs{600U};   ///< Minimum continuous hold time, in milliseconds, before
                                       ///< `LongPress` is reported.
    std::uint32_t doublePressMs{250U}; ///< Maximum gap, in milliseconds, between two releases for
                                       ///< the second to be classified as `DoublePress` rather
                                       ///< than two separate `ShortPress` gestures.
    std::uint32_t repeatDelayMs{750U}; ///< Time, in milliseconds, reserved after a press begins
                                       ///< before the first `Repeat` tick can follow a
                                       ///< `LongPress`; only takes effect once `LongPress` has
                                       ///< actually been reported.
    std::uint32_t repeatIntervalMs{120U}; ///< Interval, in milliseconds, between successive
                                          ///< `Repeat` gestures while held past `LongPress`. A
                                          ///< value of `0` disables auto-repeat entirely.
};

/**
 * @brief Classifies a debounced press/release stream into short, double, long, and repeat
 * gestures.
 */
class PressClassifier final {
  public:
    /**
     * @brief Constructs a classifier with the given timing thresholds.
     *
     * @param config Short/double/long/repeat timing configuration.
     */
    explicit PressClassifier(PressClassifierConfig config = {}) noexcept : config_(config) {}

    /**
     * @brief Updates the debounced physical state and returns at most one gesture.
     *
     * @details
     * On a press-down transition, records the start time and returns `None`; a press by itself
     * is never a completed gesture. On a release transition, completes a pending double press if
     * one was awaited within `doublePressMs`, otherwise starts the delayed single-click window
     * (see `poll()`) and returns `None`. While held, reports `LongPress` once `longPressMs` has
     * elapsed, then `Repeat` on each further `repeatIntervalMs` tick. When neither a press nor a
     * release transition nor a long-press/repeat condition applies, delegates to `poll()` so a
     * delayed single click still resolves even while the button is not currently transitioning.
     *
     * @param pressed Current debounced logical state.
     * @param nowMs Current time in milliseconds, from a monotonic clock consistent across calls.
     * @return The single gesture completed by this call, or `PressEvent::None`.
     */
    PressEvent update(bool pressed, std::uint32_t nowMs) noexcept;

    /**
     * @brief Emits a delayed short press once its double-click window has expired.
     *
     * @details
     * Intended to be called periodically (for example, from a timer tick) even when no new
     * pressed/released sample is available, so a completed single click is not held back
     * indefinitely waiting for the next `update()` call. `update()` already calls this
     * internally when it has no other gesture to report.
     *
     * @param nowMs Current time in milliseconds, from the same monotonic clock passed to
     * `update()`.
     * @return `PressEvent::ShortPress` if a pending single click's double-press window has just
     * expired; `PressEvent::None` otherwise.
     */
    PressEvent poll(std::uint32_t nowMs) noexcept;

    /**
     * @brief Resets all gesture state to match a known current physical state.
     *
     * @details
     * Discards any pending double-press or long-press/repeat tracking and re-seeds all internal
     * timestamps from `nowMs`, as if `pressed` had just begun.
     *
     * @param pressed Current debounced state to resume from.
     * @param nowMs Current time in milliseconds, from a monotonic clock consistent with later
     * `update`/`poll` calls.
     */
    void reset(bool pressed, std::uint32_t nowMs) noexcept;

  private:
    /**
     * @brief Reports whether at least `duration` has passed since a reference timestamp.
     *
     * @param now Current time in milliseconds.
     * @param since Reference timestamp in milliseconds.
     * @param duration Required elapsed time in milliseconds.
     * @return True if `now - since >= duration`, computed with wraparound-safe unsigned
     * arithmetic.
     */
    static bool elapsed(std::uint32_t now, std::uint32_t since, std::uint32_t duration) noexcept {
        return static_cast<std::uint32_t>(now - since) >= duration;
    }

    /**
     * @brief Reports whether the current time has reached a scheduled deadline.
     *
     * @param now Current time in milliseconds.
     * @param deadline Scheduled deadline in milliseconds.
     * @return True if `now` is at or past `deadline`, computed with wraparound-safe signed
     * arithmetic.
     */
    static bool reached(std::uint32_t now, std::uint32_t deadline) noexcept {
        return static_cast<std::int32_t>(now - deadline) >= 0;
    }

    PressClassifierConfig config_{}; ///< Short/double/long/repeat timing configuration.
    bool pressed_{false};            ///< Debounced state as of the most recent `update()` call.
    bool longEmitted_{false};  ///< True once `LongPress` has been reported for the current press.
    bool pendingShort_{false}; ///< True while a single click is waiting out its double-press
                               ///< window before being reported as `ShortPress`.
    std::uint32_t pressedAt_{0U};    ///< Timestamp at which the current press began.
    std::uint32_t releasedAt_{0U};   ///< Timestamp at which the pending single click was released.
    std::uint32_t nextRepeatAt_{0U}; ///< Timestamp scheduled for the next `LongPress`/`Repeat`
                                     ///< tick.
};

} // namespace eurorack::controls
