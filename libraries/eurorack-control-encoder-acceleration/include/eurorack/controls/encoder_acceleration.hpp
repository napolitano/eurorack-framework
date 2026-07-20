/**
 * @file include/eurorack/controls/encoder_acceleration.hpp
 * @brief Declares a platform-neutral encoder acceleration (ramping) policy.
 *
 * @details
 * Multiplies a raw detent count by a factor that grows as detents arrive closer together in
 * time, so fast spinning moves a value quickly while slow, deliberate turning gives fine single-
 * detent control. The class holds only a millisecond timestamp and a small config; it performs
 * no I/O and is not internally synchronized.
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
 * @brief Timing thresholds and multipliers for encoder acceleration.
 */
struct EncoderAccelerationConfig final {
    std::uint32_t fastThresholdMs{20U};   ///< Inclusive time-between-detents threshold, in
                                          ///< milliseconds, at or below which `fastMultiplier`
                                          ///< applies.
    std::uint32_t mediumThresholdMs{60U}; ///< Inclusive time-between-detents threshold, in
                                          ///< milliseconds, at or below which
                                          ///< `mediumMultiplier` applies when
                                          ///< `fastThresholdMs` was not met.
    std::int16_t fastMultiplier{8};       ///< Multiplier applied when detents arrive at or faster
                                          ///< than `fastThresholdMs` apart.
    std::int16_t mediumMultiplier{3};     ///< Multiplier applied when detents arrive at or faster
                                          ///< than `mediumThresholdMs` apart but slower than
                                          ///< `fastThresholdMs`.
};

/**
 * @brief Scales raw encoder detents by how quickly they arrive.
 *
 * @details
 * Call `apply()` once per completed detent with the current time. The first call after
 * construction or `reset()` always uses a multiplier of one, since no previous timestamp exists
 * to measure a time-between-detents interval against.
 */
class EncoderAcceleration final {
  public:
    /**
     * @brief Constructs an acceleration policy with the given timing thresholds.
     *
     * @param config Timing thresholds and multipliers.
     */
    explicit EncoderAcceleration(EncoderAccelerationConfig config = {}) noexcept
        : config_(config) {}

    /**
     * @brief Scales one detent event by how quickly it followed the previous one.
     *
     * @details
     * Returns zero without updating internal timing state when `detents` is zero. Otherwise,
     * compares the elapsed time since the previous call against `config`'s thresholds to select
     * a multiplier (one, if this is the first call since construction or `reset()`, or if the
     * elapsed time exceeds both thresholds), then always records `nowMs` for the next call.
     *
     * @param detents Signed detent count for this event; typically 1 or -1, but any value is
     * scaled the same way.
     * @param nowMs Current time in milliseconds, from a monotonic clock consistent across calls.
     * @return `detents` multiplied by the selected acceleration factor.
     */
    std::int16_t apply(std::int8_t detents, std::uint32_t nowMs) noexcept {
        if (detents == 0) {
            return 0;
        }

        std::int16_t m = 1;
        if (initialized_) {
            const auto dt = static_cast<std::uint32_t>(nowMs - lastMs_);
            if (dt <= config_.fastThresholdMs) {
                m = config_.fastMultiplier;
            } else if (dt <= config_.mediumThresholdMs) {
                m = config_.mediumMultiplier;
            }
        }

        lastMs_ = nowMs;
        initialized_ = true;
        return static_cast<std::int16_t>(detents) * m;
    }

    /**
     * @brief Forgets the recorded timestamp.
     *
     * @details
     * The next `apply()` call behaves as if called for the first time, using a multiplier of
     * one regardless of how quickly it follows the previous event.
     */
    void reset() noexcept {
        initialized_ = false;
        lastMs_ = 0U;
    }

  private:
    EncoderAccelerationConfig config_{}; ///< Timing thresholds and multipliers.
    std::uint32_t lastMs_{0U};           ///< Timestamp recorded by the previous `apply()` call.
    bool initialized_{false}; ///< True once `apply()` has run at least once since construction
                              ///< or the last `reset()` call.
};

} // namespace eurorack::controls
