/**
 * @file include/eurorack/controls/rotary_encoder.hpp
 * @brief Quadrature rotary encoder decoder.
 *
 *
 * @details
 * Control objects translate raw hardware observations into stable, application-facing state.
 * They do not own referenced hardware interfaces; dependencies must outlive the control object.
 * Unless stated otherwise, calls are synchronous, allocate no memory, and are intended for a
 * cooperative firmware loop. Objects are not internally synchronized and require external
 * protection when shared between interrupt and foreground contexts.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * @see LICENSE and ADDITIONAL_PERMISSION.md in the repository root.
 *
 * @ingroup controls
 */
#pragma once
#include <cstdint>
namespace eurorack::controls {
/**
 * @brief Direction represented by the most recent completed encoder detent.
 *
 * @details
 * `None` is reported when the latest sample produced no complete detent or when
 * an invalid transition was rejected.
 */
enum class RotationDirection : std::int8_t { None = 0, CounterClockwise = -1, Clockwise = 1 };
/**
 * @brief Resolution and logical direction settings for a quadrature encoder.
 *
 * @details
 * `transitionsPerDetent` must match the mechanical encoder and sampling method.
 * Four is typical when every Gray-code edge is sampled. One may be appropriate
 * when a hardware decoder already reports complete detents.
 */
struct RotaryEncoderConfig final {
    std::uint8_t transitionsPerDetent{4U};
    bool inverted{false};
};
/**
 * @brief Application-facing position and diagnostic state of a rotary encoder.
 *
 * @details
 * `delta` is the movement completed by the most recent update and is therefore
 * transient. `position` accumulates completed detents. Invalid transitions are
 * counted so that missed samples, bounce, and wiring errors remain observable.
 */
struct RotaryEncoderSnapshot final {
    std::int32_t position{0};
    std::int32_t delta{0};
    RotationDirection direction{RotationDirection::None};
    std::uint32_t invalidTransitionCount{0U};
};
/**
 * @brief Decodes sampled quadrature A/B levels into detents and direction.
 *
 * @details
 * The decoder uses the legal Gray-code transition table and rejects impossible
 * two-bit jumps. Call update() whenever either channel may have changed. Slow
 * polling can miss intermediate states and will increase
 * `invalidTransitionCount`. The class performs no GPIO access, allocates no
 * memory, and requires external synchronization between ISR and foreground use.
 */
class RotaryEncoder final {
  public:
    /** @brief Constructs an encoder decoder. @param config Resolution and direction. */
    explicit RotaryEncoder(RotaryEncoderConfig config = {}) noexcept;
    /** @brief Resets decoder state. @param levelA Channel A level. @param levelB Channel B level.
     * @param position Initial position. */
    void reset(bool levelA, bool levelB, std::int32_t position = 0) noexcept;
    /** @brief Processes sampled A/B levels. @param levelA Channel A level. @param levelB Channel B
     * level. */
    void update(bool levelA, bool levelB) noexcept;
    /** @brief Returns current encoder state. @return Constant state reference. */
    [[nodiscard]] const RotaryEncoderSnapshot& snapshot() const noexcept;

  private:
    RotaryEncoderConfig config_{};
    RotaryEncoderSnapshot snapshot_{};
    bool initialized_{false};
    std::uint8_t previousState_{0U};
    std::int8_t accumulator_{0};
};
} // namespace eurorack::controls
