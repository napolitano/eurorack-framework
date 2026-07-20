/**
 * @file include/eurorack/platform/avr/median_of_three_scanner.hpp
 * @brief Declares a hardware-independent, three-channel round-robin median-of-three scan model.
 *
 * @details
 * Holds exactly the algorithmic state of `AdcScanner3`: a rolling three-sample window per
 * channel, round-robin rotation across three channels, a one-sample discard after every channel
 * switch, and a "ready" flag that becomes true once every channel has filled its window at least
 * once. Nothing in this class touches a register, an ADC peripheral, or any AVR-specific
 * facility, so it compiles and runs identically on a desktop build and is exercised directly by
 * native unit tests. `AdcScanner3` wraps one instance of this class and is the only part of that
 * pair that talks to real ADC hardware; when `recordSample()` reports that the active channel
 * just changed, `AdcScanner3` is the one that reprograms the ADC multiplexer, since this model
 * has no concept of `ADMUX` or any other register.
 *
 * @ingroup platform_avr
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
#include <eurorack/compat/avr/cstddef.hpp>
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstddef>
#include <cstdint>
#endif

namespace eurorack::platform::avr {

/**
 * @brief Median-filtered snapshot of the three scanned channels.
 */
struct MedianOfThreeSnapshot final {
    std::uint16_t channel[3]{0U, 0U, 0U}; ///< Median-of-three raw sample for each channel index.
    bool ready{false}; ///< True once at least one full rotation through all three channels has
                       ///< completed; the medians are not yet meaningful while false.
};

/**
 * @brief Reports whether `MedianOfThreeScanner::recordSample` just rotated to a new channel.
 */
struct MedianOfThreeChannelSwitch final {
    bool changed{false};      ///< True if the active channel just advanced.
    std::uint8_t channel{0U}; ///< The newly active channel index; only meaningful if `changed`.
};

/**
 * @brief Round-robin, median-of-three sampling model for exactly three channels.
 *
 * @details
 * Not internally synchronized: a caller sharing an instance between an interrupt handler and
 * mainline code is responsible for making each individual method call atomic with respect to the
 * other context, exactly as `AdcScanner3` does with `ATOMIC_BLOCK` around every call from
 * mainline code.
 */
class MedianOfThreeScanner final {
  public:
    static constexpr std::size_t channelCount = 3U; ///< Fixed number of scanned channels.

    /**
     * @brief Records one sample for the currently active channel.
     *
     * @details
     * Discards the first sample after every channel switch without recording it, since the
     * caller's sample-and-hold hardware has not yet settled to a newly selected channel. Once
     * three samples have been recorded for the active channel, advances to the next channel
     * (wrapping after the last one back to the first, at which point the "ready" state becomes
     * true) and reports the new channel so the caller can reprogram its hardware multiplexer.
     *
     * @param value Raw sample for the currently active channel.
     * @return A switch report: `changed` is true only on the call that completes a channel's
     * window and advances to the next one, in which case `channel` holds the newly active index.
     */
    MedianOfThreeChannelSwitch recordSample(std::uint16_t value) noexcept {
        if (discard_ > 0U) {
            discard_ = 0U;
            return {};
        }

        samples_[channel_][sampleIndex_] = value;
        ++sampleIndex_;

        if (sampleIndex_ < 3U) {
            return {};
        }

        sampleIndex_ = 0U;
        ++channel_;
        if (channel_ >= channelCount) {
            channel_ = 0U;
            ready_ = true;
        }
        discard_ = 1U;
        return {true, channel_};
    }

    /**
     * @brief Returns the current median-filtered snapshot.
     *
     * @return Snapshot with the median of the last three samples for each channel, and whether a
     * full rotation has completed at least once.
     */
    [[nodiscard]] MedianOfThreeSnapshot snapshot() const noexcept {
        MedianOfThreeSnapshot result;
        for (std::size_t i = 0U; i < channelCount; ++i) {
            result.channel[i] = median(samples_[i][0], samples_[i][1], samples_[i][2]);
        }
        result.ready = ready_;
        return result;
    }

    /**
     * @brief Resets all scan state to its post-construction values.
     *
     * @details
     * The next `recordSample()` call is treated as the first sample of a fresh channel-zero
     * window and is discarded, exactly as after construction.
     */
    void reset() noexcept {
        for (auto& row : samples_) {
            row[0] = row[1] = row[2] = 0U;
        }
        channel_ = 0U;
        sampleIndex_ = 0U;
        discard_ = 1U;
        ready_ = false;
    }

  private:
    /**
     * @brief Returns the median of three values.
     *
     * @param a First value.
     * @param b Second value.
     * @param c Third value.
     * @return The middle value once `a`, `b`, and `c` are considered in sorted order.
     */
    static std::uint16_t median(std::uint16_t a, std::uint16_t b, std::uint16_t c) noexcept {
        if (a > b) {
            const std::uint16_t t = a;
            a = b;
            b = t;
        }
        if (b > c) {
            const std::uint16_t t = b;
            b = c;
            c = t;
        }
        if (a > b) {
            const std::uint16_t t = a;
            a = b;
            b = t;
        }
        return b;
    }

    std::uint16_t samples_[3][3]{}; ///< Rolling three-sample window per channel, indexed
                                    ///< `[channel][sampleIndex]`.
    std::uint8_t channel_{0U};      ///< Currently active channel index.
    std::uint8_t sampleIndex_{0U};  ///< Next slot to fill within the active channel's window.
    std::uint8_t discard_{1U}; ///< Nonzero while the next sample for the active channel must be
                               ///< discarded rather than recorded.
    bool ready_{false}; ///< True once every channel has filled its rolling window at least once.
};

} // namespace eurorack::platform::avr
