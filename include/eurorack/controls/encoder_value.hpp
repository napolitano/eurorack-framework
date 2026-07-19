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
    std::int32_t minimum{0}; ///< Lower bound, inclusive. Swapped with `maximum` by the
                              ///< constructor if it is greater than `maximum`.
    std::int32_t maximum{127}; ///< Upper bound, inclusive. Swapped with `minimum` by the
                                ///< constructor if it is smaller than `minimum`.
    std::int32_t step{1}; ///< Value change per detent. The constructor replaces `0` with `1`;
                            ///< negative values reverse the encoder's effective direction.
    EncoderBoundaryMode boundaryMode{EncoderBoundaryMode::Clamp}; ///< Behavior applied when a
                                                                    ///< candidate value leaves the
                                                                    ///< configured range; see
                                                                    ///< @ref EncoderValue::normalize.
};

/**
 * @brief Immutable encoder-driven value state.
 */
struct EncoderValueSnapshot final {
    std::int32_t value{0}; ///< Current bounded value.
    std::int32_t delta{0}; ///< Signed change applied by the most recent call to
                             ///< @ref EncoderValue::applyDetents, after clamping or wrapping.
    bool changed{false}; ///< True when the most recent @ref EncoderValue::applyDetents call
                          ///< produced a different `value`; false after @ref EncoderValue::reset.
    bool wrapped{false}; ///< True when the most recent candidate value crossed a bound while
                          ///< `boundaryMode` is `Wrap`.
    bool clamped{false}; ///< True when the most recent candidate value crossed a bound while
                          ///< `boundaryMode` is `Clamp`.
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

    EncoderValueConfig config_{};     ///< Bounds, step size, and boundary mode; `minimum` and
                                        ///< `maximum` are already ordered and `step` is non-zero.
    EncoderValueSnapshot snapshot_{}; ///< Most recently calculated value and event flags.
};

} // namespace eurorack::controls
