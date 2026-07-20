/**
 * @file include/eurorack/controls/analog_button_ladder.hpp
 * @brief Declares a deterministic decoder for multiple buttons sharing one resistor-ladder ADC
 * input.
 *
 * @details
 * A resistor ladder wires several panel buttons to a single ADC pin, each through a different
 * resistor value, so that pressing a given button pulls the pin to a roughly known voltage code.
 * This class decodes a raw ADC sample into a button index by nearest-code matching, rejects
 * samples that do not plausibly match any configured code or the open (no button pressed) state,
 * and only commits a new pressed state once the decoded candidate has been stable for a
 * configured minimum time, filtering out the brief in-between codes produced while a physical
 * button is transitioning. The class performs no I/O and holds no hardware reference; the caller
 * supplies raw ADC samples and owns the expected-code table.
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
#include <eurorack/compat/avr/cstddef.hpp>
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstddef>
#include <cstdint>
#endif

namespace eurorack::controls {

/**
 * @brief Expected ADC codes and decoding tolerances for one resistor-ladder button bank.
 */
struct AnalogButtonLadderConfig final {
    const std::uint16_t* expectedCodes{nullptr}; ///< Caller-owned array of `buttonCount` expected
                                                 ///< ADC codes, one per button, indexed the
                                                 ///< same way as the decoded button index; must
                                                 ///< outlive the `AnalogButtonLadder`.
    std::size_t buttonCount{0U};                 ///< Number of entries in `expectedCodes`.
    std::uint16_t openThreshold{1023U};          ///< Inclusive lower bound above which a sample is
                                        ///< decoded as "no button pressed" rather than matched
                                        ///< against `expectedCodes`.
    std::uint16_t acceptanceWindow{16U}; ///< Maximum absolute distance between a sample and its
                                         ///< nearest expected code for that code to be
                                         ///< accepted; a larger gap decodes as an invalid
                                         ///< sample.
    std::uint32_t stableTimeMs{64U};     ///< Minimum time a decoded candidate must remain unchanged
                                         ///< before `update()` commits it as the new pressed state.
};

/**
 * @brief Decoded button state and edge flags produced by `AnalogButtonLadder`.
 */
struct AnalogButtonLadderSnapshot final {
    std::int16_t pressedIndex{-1}; ///< Currently committed pressed button index, or `-1` when no
                                   ///< button is pressed.
    std::int16_t justPressedIndex{-1};  ///< Button index that became pressed on the most recent
                                        ///< `update()` call, or `-1` if none did.
    std::int16_t justReleasedIndex{-1}; ///< Button index that was released on the most recent
                                        ///< `update()` call, or `-1` if none was.
    bool validSample{true}; ///< True when the most recent raw sample decoded to either a known
                            ///< button or the open state; false when it fell outside every
                            ///< acceptance window and could not be decoded.
};

/**
 * @brief Debounced decoder for a bank of buttons sharing one resistor-ladder ADC input.
 */
class AnalogButtonLadder final {
  public:
    /**
     * @brief Constructs a decoder for a given resistor-ladder configuration.
     *
     * @param config Expected codes, thresholds, and debounce time. `config.expectedCodes` must
     * outlive this object.
     */
    explicit AnalogButtonLadder(AnalogButtonLadderConfig config) noexcept : config_(config) {}

    /**
     * @brief Immediately initializes the decoder from one sample, bypassing debounce.
     *
     * @details
     * Decodes `raw` and commits it as the current pressed state right away, without waiting for
     * `stableTimeMs`. Does not set `justPressedIndex`/`justReleasedIndex`, since there is no
     * previous state to transition from. `update()` calls this automatically on its first
     * invocation.
     *
     * @param raw ADC code to decode as the initial state.
     * @param nowMs Current time in milliseconds, from a monotonic clock consistent with later
     * `update()` calls.
     */
    void reset(std::uint16_t raw, std::uint32_t nowMs) noexcept;

    /**
     * @brief Processes one ADC sample and updates the debounced pressed state.
     *
     * @details
     * Clears the previous call's edge flags first. If the sample decodes to a different button
     * (or open state) than the current debounce candidate, that becomes the new candidate and
     * its timer restarts; a candidate is only committed to `pressedIndex` once it has remained
     * unchanged for at least `stableTimeMs`. Invalid samples (see `AnalogButtonLadderSnapshot`)
     * are recorded but otherwise ignored, leaving the debounce state unchanged.
     *
     * @param raw ADC code sampled from the resistor ladder.
     * @param nowMs Current time in milliseconds, from a monotonic clock consistent with `reset`
     * and previous `update` calls.
     */
    void update(std::uint16_t raw, std::uint32_t nowMs) noexcept;

    /**
     * @brief Returns the current decoded state.
     *
     * @return Constant reference to the latest committed pressed state and edge flags.
     */
    [[nodiscard]] const AnalogButtonLadderSnapshot& snapshot() const noexcept {
        return snapshot_;
    }

  private:
    /**
     * @brief Decodes a raw sample into a button index by nearest-code matching.
     *
     * @param raw ADC code to decode.
     * @param valid Receives true if `raw` plausibly matches the open state or one configured
     * button code within `acceptanceWindow`; false otherwise.
     * @return Matched button index, or `-1` for the open state or an invalid sample.
     */
    [[nodiscard]] std::int16_t decode(std::uint16_t raw, bool& valid) const noexcept;

    AnalogButtonLadderConfig config_{};     ///< Expected codes, thresholds, and debounce time.
    AnalogButtonLadderSnapshot snapshot_{}; ///< Most recently committed state and edge flags.
    std::int16_t candidate_{-1}; ///< Decoded button index currently being debounced; `-1` for
                                 ///< the open state.
    std::uint32_t candidateSinceMs_{0U}; ///< Timestamp at which `candidate_` last changed.
    bool initialized_{false}; ///< True once `reset()` has run at least once; `update()` calls
                              ///< `reset()` on its first invocation.
};

} // namespace eurorack::controls
