/**
 * @file include/eurorack/simulation/virtual_io.hpp
 * @brief Declares virtual digital and analog hardware channels.
 *
 * @details
 * Provides inspectable input and output channels for tests and desktop simulation.
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
 * @ingroup simulation
 */

#if !defined(ARDUINO)

#pragma once

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/io/analog_io.hpp>
#include <eurorack/io/digital_io.hpp>

namespace eurorack::simulation {

/**
 * @brief Virtual digital input whose logical level is set directly by test code.
 *
 * @details
 * Provides no timing or debounce; `readHigh` simply returns whatever `setHigh` (or the
 * constructor) last stored.
 */
class VirtualDigitalInput final : public eurorack::io::DigitalInput {
  public:
    /**
     * @brief Constructs a virtual digital input at a given initial level.
     *
     * @param high Initial logical level returned by `readHigh`.
     */
    explicit VirtualDigitalInput(bool high = false) noexcept;

    /**
     * @brief Reads the current logical state.
     *
     * @return The level last set via `setHigh` or the constructor.
     */
    [[nodiscard]] bool readHigh() const noexcept override;

    /**
     * @brief Sets the logical state returned by subsequent `readHigh` calls.
     *
     * @param high Logical state to apply.
     */
    void setHigh(bool high) noexcept;

  private:
    bool high_{false}; ///< Logical level returned by `readHigh`.
};

/**
 * @brief Virtual digital output that records the last written level and transition count.
 */
class VirtualDigitalOutput final : public eurorack::io::DigitalOutput {
  public:
    /**
     * @brief Constructs a virtual digital output at a given initial level.
     *
     * @param high Initial level reported by `lastWrittenHigh`; does not count as a transition.
     */
    explicit VirtualDigitalOutput(bool high = false) noexcept;

    /**
     * @brief Writes one logical level.
     *
     * @details
     * Increments `transitionCount` only when `high` differs from the previously written level.
     *
     * @param high Logical state to apply.
     */
    void writeHigh(bool high) noexcept override;

    /**
     * @brief Returns the last logical output state.
     *
     * @return The level last passed to `writeHigh` or the constructor.
     */
    [[nodiscard]] bool lastWrittenHigh() const noexcept override;

    /**
     * @brief Returns the number of output-state transitions.
     *
     * @return Count of `writeHigh` calls whose level differed from the previous level, since
     * construction.
     */
    [[nodiscard]] std::uint32_t transitionCount() const noexcept;

  private:
    bool high_{false};                  ///< Level last passed to `writeHigh` or the constructor.
    std::uint32_t transitionCount_{0U}; ///< Number of accepted level changes since construction.
};

/**
 * @brief Virtual bidirectional digital pin with separate output and externally driven levels.
 *
 * @details
 * Tracks an output level (set via `writeHigh`) and an externally observed level (set via
 * `setExternalHigh`) independently; `readHigh` returns whichever one is relevant for the
 * currently configured direction.
 */
class VirtualBidirectionalPin final : public eurorack::io::BidirectionalDigitalPin {
  public:
    /**
     * @brief Constructs a pin defaulting to input direction with both levels low.
     */
    VirtualBidirectionalPin() noexcept = default;

    /**
     * @brief Changes the digital pin direction.
     *
     * @param direction Requested input or output direction; affects which level `readHigh`
     * reports.
     */
    void setDirection(eurorack::io::BidirectionalDigitalPin::Direction direction) noexcept override;

    /**
     * @brief Returns the currently configured direction.
     *
     * @return Direction last passed to `setDirection`.
     */
    [[nodiscard]] eurorack::io::BidirectionalDigitalPin::Direction
    direction() const noexcept override;

    /**
     * @brief Reads the current logical state.
     *
     * @details
     * Returns the output level (as last set via `writeHigh`) when direction is `Output`, or the
     * externally observed level (as last set via `setExternalHigh`) when direction is `Input`.
     *
     * @return Direction-dependent logical level.
     */
    [[nodiscard]] bool readHigh() const noexcept override;

    /**
     * @brief Sets the output level, independently of the currently configured direction.
     *
     * @param high Logical state to apply.
     */
    void writeHigh(bool high) noexcept override;

    /**
     * @brief Returns the last output level written.
     *
     * @return Level last passed to `writeHigh`, regardless of direction.
     */
    [[nodiscard]] bool lastWrittenHigh() const noexcept override;

    /**
     * @brief Sets the externally observed level used when direction is `Input`.
     *
     * @param high Logical level to simulate as driven onto the pin from outside.
     */
    void setExternalHigh(bool high) noexcept;

  private:
    eurorack::io::BidirectionalDigitalPin::Direction direction_{
        eurorack::io::BidirectionalDigitalPin::Direction::Input}; ///< Currently configured
                                                                  ///< direction.
    bool outputHigh_{false};   ///< Level last passed to `writeHigh`.
    bool externalHigh_{false}; ///< Level last passed to `setExternalHigh`.
};

/**
 * @brief Virtual ADC input channel with a test-controlled code and injectable result.
 */
class VirtualAnalogInput final : public eurorack::io::AnalogInputChannel {
  public:
    /**
     * @brief Constructs a virtual analog input.
     *
     * @param maximumCode Inclusive maximum raw converter code accepted by `setCode`.
     */
    explicit VirtualAnalogInput(std::uint32_t maximumCode = 4095U) noexcept;

    /**
     * @brief Reads one raw analog sample.
     *
     * @return The code last passed to `setCode` (or `0` if never set), paired with the result
     * last passed to `setResult` (or `Success` if never set). Unlike the virtual bus classes,
     * the injected result is not consumed; it persists until changed again.
     */
    [[nodiscard]] eurorack::io::AnalogSample readRaw() noexcept override;

    /**
     * @brief Returns the inclusive maximum converter code.
     *
     * @return Maximum code, as passed to the constructor.
     */
    [[nodiscard]] std::uint32_t maximumCode() const noexcept override;

    /**
     * @brief Sets the raw code returned by subsequent `readRaw` calls.
     *
     * @details
     * Silently clamps `code` to `maximumCode()`.
     *
     * @param code Raw converter code.
     */
    void setCode(std::uint32_t code) noexcept;

    /**
     * @brief Sets the result returned by subsequent `readRaw` calls.
     *
     * @details
     * The result persists across calls until changed again by another `setResult` call; it is
     * not automatically reset to `Success`.
     *
     * @param result Result returned by subsequent `readRaw` calls.
     */
    void setResult(eurorack::io::IoResult result) noexcept;

  private:
    std::uint32_t maximumCode_{4095U}; ///< Inclusive maximum code; `setCode` clamps to this
                                       ///< value.
    std::uint32_t code_{0U};           ///< Code returned by `readRaw`.
    eurorack::io::IoResult result_{
        eurorack::io::IoResult::Success}; ///< Result returned by
                                          ///< `readRaw`; persists until changed via `setResult`.
};

/**
 * @brief Virtual DAC output channel that validates codes and can inject a sticky result.
 */
class VirtualAnalogOutput final : public eurorack::io::AnalogOutputChannel {
  public:
    /**
     * @brief Constructs a virtual analog output.
     *
     * @param maximumCode Inclusive maximum raw converter code accepted by `writeRaw`.
     */
    explicit VirtualAnalogOutput(std::uint32_t maximumCode = 4095U) noexcept;

    /**
     * @brief Writes one raw analog output code.
     *
     * @details
     * Returns `InvalidArgument` without changing `lastWrittenCode()` if `code` exceeds
     * `maximumCode()`. Otherwise, `code` is stored as the new `lastWrittenCode()` only if the
     * currently injected result (see `setResult`) is `Success`; if a non-`Success` result is
     * injected, that result is returned and the code is silently discarded.
     *
     * @param code Raw converter code.
     * @return `InvalidArgument` if `code` exceeds `maximumCode()`; otherwise the result last
     * passed to `setResult` (defaulting to `Success`).
     */
    eurorack::io::IoResult writeRaw(std::uint32_t code) noexcept override;

    /**
     * @brief Returns the last accepted raw output code.
     *
     * @return Code from the most recent `writeRaw` call that was within range and accepted
     * while the injected result was `Success`; `0` if none has been accepted yet.
     */
    [[nodiscard]] std::uint32_t lastWrittenCode() const noexcept override;

    /**
     * @brief Returns the inclusive maximum converter code.
     *
     * @return Maximum code, as passed to the constructor.
     */
    [[nodiscard]] std::uint32_t maximumCode() const noexcept override;

    /**
     * @brief Sets the result returned by subsequent `writeRaw` calls for in-range codes.
     *
     * @details
     * The result persists across calls until changed again by another `setResult` call; it is
     * not automatically reset to `Success`. While non-`Success`, `writeRaw` returns it without
     * updating `lastWrittenCode()`.
     *
     * @param result Result returned by subsequent in-range `writeRaw` calls.
     */
    void setResult(eurorack::io::IoResult result) noexcept;

  private:
    std::uint32_t maximumCode_{4095U}; ///< Inclusive maximum code; `writeRaw` rejects anything
                                       ///< above this with `InvalidArgument`.
    std::uint32_t code_{0U};           ///< Last code accepted by `writeRaw`.
    eurorack::io::IoResult result_{
        eurorack::io::IoResult::Success}; ///< Result returned by
                                          ///< `writeRaw` for in-range codes; persists until changed
                                          ///< via `setResult`.
};

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
