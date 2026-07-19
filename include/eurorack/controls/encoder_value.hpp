/**
 * @file include/eurorack/controls/encoder_value.hpp
 * @brief Declares a bounded value model driven by relative encoder steps.
 *
 * @details
 * Converts relative detents into a clamped or wrapped value with configurable step size.
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
 * @ingroup controls
 */

#pragma once

#include <cstdint>

namespace eurorack::controls {

/**
 * @brief Behavior when an encoder-driven value crosses its bounds.
 */
enum class EncoderBoundaryMode : std::uint8_t { Clamp, Wrap };

/**
 * @brief Configuration for an encoder-driven integer value.
 */
struct EncoderValueConfig final {
    std::int32_t minimum{0};
    std::int32_t maximum{127};
    std::int32_t step{1};
    EncoderBoundaryMode boundaryMode{EncoderBoundaryMode::Clamp};
};

/**
 * @brief Immutable encoder-driven value state.
 */
struct EncoderValueSnapshot final {
    std::int32_t value{0};
    std::int32_t delta{0};
    bool changed{false};
    bool wrapped{false};
    bool clamped{false};
};

/**
 * @brief Converts relative encoder detents into a bounded integer value.
 */
class EncoderValue final {
  public:
    /**
     * @brief Constructs a bounded encoder value.
     *
     * @param config Bounds, step size, and boundary behavior.
     * @param initialValue Initial value before range normalization.
     */
    EncoderValue(EncoderValueConfig config = {}, std::int32_t initialValue = 0) noexcept;

    /**
     * @brief Resets the current value.
     *
     * @param value New value before range normalization.
     */
    void reset(std::int32_t value) noexcept;

    /**
     * @brief Applies relative encoder detents.
     *
     * @param detents Signed detent count.
     */
    void applyDetents(std::int32_t detents) noexcept;

    /**
     * @brief Returns the current immutable value state.
     *
     * @return Constant reference to current value and event flags.
     */
    [[nodiscard]] const EncoderValueSnapshot& snapshot() const noexcept;

  private:
    /**
     * @brief Normalizes one candidate value according to configured bounds.
     *
     * @param candidate Unbounded candidate value.
     * @return Bounded value.
     */
    [[nodiscard]] std::int32_t normalize(std::int64_t candidate) noexcept;

    EncoderValueConfig config_{};
    EncoderValueSnapshot snapshot_{};
};

} // namespace eurorack::controls
