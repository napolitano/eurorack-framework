/**
 * @file include/eurorack/controls/bicolor_led.hpp
 * @brief Platform-independent two-color LED model.
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
 * @brief Electrical topology of a two-color LED package.
 *
 * @details
 * Common-anode and common-cathode packages expose one shared terminal and two
 * independently driven color terminals. Bipolar two-lead packages reverse
 * current direction to select a color and cannot display a static mixed color
 * without time multiplexing.
 */
enum class BiColorLedTopology : std::uint8_t { CommonAnode, CommonCathode, Bipolar };
/**
 * @brief Logical color requested from a two-color LED.
 *
 * @details
 * `Mixed` means that both color dies should contribute. For a bipolar
 * two-terminal device this requires alternating the current direction; the
 * snapshot therefore reports `multiplexingRequired`.
 */
enum class BiColorLedColor : std::uint8_t { Off, ColorA, ColorB, Mixed };
/**
 * @brief Electrical drive state required on one LED terminal.
 *
 * @details
 * `HighImpedance` means that the hardware adapter should configure the terminal
 * as an input or otherwise disconnect it. This is required to avoid unintended
 * current paths for some LED topologies.
 */
enum class PinDrive : std::uint8_t { Low, High, HighImpedance };
/**
 * @brief Immutable wiring and startup configuration for a two-color LED.
 *
 * @details
 * The configuration describes the package topology only. Series resistors,
 * current limits, PWM frequency, optical calibration, and GPIO ownership remain
 * responsibilities of the hardware layer.
 */
struct BiColorLedConfig final {
    BiColorLedTopology topology{BiColorLedTopology::CommonAnode};
    BiColorLedColor initialColor{BiColorLedColor::Off};
};
/**
 * @brief Complete logical and electrical state calculated for a two-color LED.
 *
 * @details
 * Application code normally consumes `requestedColor` and `effectiveColor`.
 * A hardware adapter consumes `pinA`, `pinB`, and `multiplexingRequired`.
 * Transition metadata describes accepted logical changes, not PWM edges.
 */
struct BiColorLedSnapshot final {
    BiColorLedColor requestedColor{BiColorLedColor::Off};
    BiColorLedColor effectiveColor{BiColorLedColor::Off};
    PinDrive pinA{PinDrive::HighImpedance};
    PinDrive pinB{PinDrive::HighImpedance};
    bool multiplexingRequired{false};
    bool changed{false};
    std::uint32_t transitionCount{0U};
};
/**
 * @brief Converts a logical two-color request into topology-specific pin states.
 *
 * @details
 * The class is a pure state model: it owns no GPIO objects and performs no
 * timing. For a bipolar LED in `Mixed` mode, the caller must provide a
 * multiplexing scheduler and alternate the two color drive states fast enough
 * to avoid visible flicker. The object allocates no memory and is not internally
 * synchronized.
 */
class BiColorLed final {
  public:
    /** @brief Constructs the LED model. @param config Wiring topology and initial color. */
    explicit BiColorLed(BiColorLedConfig config = {}) noexcept;
    /** @brief Restores the configured initial color and clears history. */
    void reset() noexcept;
    /** @brief Requests a logical color. @param color Requested color. */
    void setColor(BiColorLedColor color) noexcept;
    /** @brief Returns current logical and electrical state. @return Constant state reference. */
    [[nodiscard]] const BiColorLedSnapshot& snapshot() const noexcept;
    /** @brief Returns the configured wiring topology. @return LED topology. */
    [[nodiscard]] BiColorLedTopology topology() const noexcept;

  private:
    /** @brief Recalculates effective color and required pin drives. */
    void recalculate() noexcept;
    BiColorLedConfig config_{};
    BiColorLedSnapshot snapshot_{};
};
} // namespace eurorack::controls
