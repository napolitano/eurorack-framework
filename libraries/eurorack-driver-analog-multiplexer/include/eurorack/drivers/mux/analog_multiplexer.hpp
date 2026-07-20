/**
 * @file include/eurorack/drivers/mux/analog_multiplexer.hpp
 * @brief Declares a driver for binary-addressed analog multiplexers (e.g. 74HC4051/74HC4067).
 *
 * @details
 * Wraps a bank of digital select-line outputs and one shared analog input channel behind the
 * framework's `AnalogInputChannel` interface, so a single ADC pin can serve several logical
 * inputs (typically several potentiometers or CV jacks on an MCU with few ADC pins, such as the
 * ATmega328P). The channel count is a compile-time template parameter so the driver needs no
 * dynamic allocation and holds no more state than the select-line count requires.
 *
 * Channel selection follows the standard binary addressing of 4051-family multiplexers: channel
 * index bit 0 drives the first select line, bit 1 the second, and so on. The driver does not
 * drive an inhibit/enable pin; tie it according to your circuit (permanently enabled, or driven
 * externally).
 *
 * The select-line outputs, the common analog input, and the delay provider are non-owning
 * dependencies. Calls are synchronous, allocate no memory, and are not internally synchronized.
 * Electrical protection, mux propagation delay margin beyond the configured settle time, and pin
 * assignment remain outside this class.
 *
 * @ingroup drivers
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
#include <eurorack/compat/avr/array.hpp>
#else
#include <array>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <cstddef>
#endif
#include <eurorack/io/analog_io.hpp>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/time_source.hpp>

namespace eurorack::drivers::mux {

/**
 * @brief Binary-addressed analog multiplexer with `SelectLineCount` select lines.
 *
 * @tparam SelectLineCount Number of binary select lines (3 for an 8-channel 74HC4051, 4 for a
 * 16-channel 74HC4067). `channelCount` is `2^SelectLineCount`.
 */
template <std::size_t SelectLineCount>
class AnalogMultiplexer final : public eurorack::io::AnalogInputChannel {
  public:
    /** @brief Number of logical channels this multiplexer exposes. */
    static constexpr std::size_t channelCount = std::size_t{1} << SelectLineCount;

    /**
     * @brief Constructs a multiplexer driver over existing select-line and ADC dependencies.
     *
     * @param selectLines Select-line outputs, ordered from the least significant address bit
     * (index 0) to the most significant (index `SelectLineCount - 1`); the driver does not own
     * them.
     * @param commonInput Shared analog input channel connected to the multiplexer's common
     * (`Z`) pin; the driver does not own it.
     * @param delay Blocking delay service used to wait out the multiplexer's propagation delay
     * after switching channels; the driver does not own it.
     * @param settleMicroseconds Minimum time to wait after switching select lines before the
     * common input is sampled; chosen for the specific multiplexer part and source impedance in
     * use, not derived from a datasheet default by this class.
     */
    AnalogMultiplexer(std::array<eurorack::io::DigitalOutput*, SelectLineCount> selectLines,
                      eurorack::io::AnalogInputChannel& commonInput,
                      eurorack::io::DelayProvider& delay,
                      std::uint32_t settleMicroseconds) noexcept
        : selectLines_(selectLines), commonInput_(commonInput), delay_(delay),
          settleMicroseconds_(settleMicroseconds) {}

    /**
     * @brief Drives the select lines for one channel and waits out the settle time.
     *
     * @details
     * Channel bits are written most-significant-first; a null entry in `selectLines` for a given
     * bit position is skipped without error, which only makes sense if that address bit is fixed
     * externally (for example, tied to ground) rather than driven by this instance.
     *
     * @param channel Zero-based channel index; values at or above `channelCount` are masked to
     * their low `SelectLineCount` bits rather than rejected.
     */
    void selectChannel(const std::size_t channel) noexcept {
        for (std::size_t line = 0U; line < SelectLineCount; ++line) {
            if (selectLines_[line] != nullptr) {
                const bool bitSet = ((channel >> line) & 0x1U) != 0U;
                selectLines_[line]->writeHigh(bitSet);
            }
        }

        currentChannel_ = channel & (channelCount - 1U);
        delay_.delayMicroseconds(settleMicroseconds_);
    }

    /**
     * @brief Reads the common analog input, assuming a channel has already been selected.
     *
     * @details
     * Does not switch channels or wait for settling; call `selectChannel` or `readChannel`
     * first. Exists so this class satisfies `AnalogInputChannel` for framework code that expects
     * a plain input channel already pointed at one fixed logical input.
     *
     * @return Raw code and acquisition status from `commonInput`.
     */
    [[nodiscard]] eurorack::io::AnalogSample readRaw() noexcept override {
        return commonInput_.readRaw();
    }

    /**
     * @brief Returns the maximum converter code of the underlying common input.
     *
     * @return Inclusive maximum code, as reported by `commonInput`.
     */
    [[nodiscard]] std::uint32_t maximumCode() const noexcept override {
        return commonInput_.maximumCode();
    }

    /**
     * @brief Selects one channel, waits out the settle time, and reads it.
     *
     * @param channel Zero-based channel index; values at or above `channelCount` are masked to
     * their low `SelectLineCount` bits rather than rejected.
     * @return Raw code and acquisition status from `commonInput`.
     */
    [[nodiscard]] eurorack::io::AnalogSample readChannel(const std::size_t channel) noexcept {
        selectChannel(channel);
        return commonInput_.readRaw();
    }

    /**
     * @brief Returns the channel selected by the most recent `selectChannel`/`readChannel` call.
     *
     * @return Zero-based channel index; `0` before the first selection.
     */
    [[nodiscard]] std::size_t currentChannel() const noexcept {
        return currentChannel_;
    }

  private:
    std::array<eurorack::io::DigitalOutput*, SelectLineCount>
        selectLines_; ///< Select-line
                      ///< outputs, least significant address bit first.
    eurorack::io::AnalogInputChannel& commonInput_; ///< Shared ADC channel on the mux's common
                                                    ///< pin.
    eurorack::io::DelayProvider& delay_;   ///< Blocking delay used after each channel switch.
    std::uint32_t settleMicroseconds_{0U}; ///< Configured post-switch settle time.
    std::size_t currentChannel_{0U};       ///< Channel selected by the last switch.
};

} // namespace eurorack::drivers::mux
