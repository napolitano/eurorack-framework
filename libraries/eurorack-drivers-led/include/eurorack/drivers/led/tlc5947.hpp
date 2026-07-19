/**
 * @file include/eurorack/drivers/led/tlc5947.hpp
 * @brief Declares a buffered driver for cascaded TLC5947 24-channel PWM sinks.
 *
 * @details
 * Each TLC5947 contributes twenty-four 12-bit PWM channels. The generic 16-bit
 * brightness value is reduced to 12 bits when the frame is built. Channels are
 * serialized in the order required by a cascaded chain and latched together.
 *
 * Buffers are allocated during construction. The SPI bus, latch output, and
 * optional output-enable output are non-owning dependencies.
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
#include <eurorack/compat/avr/cstddef.hpp>
#else
#include <cstddef>
#endif
#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/drivers/led/led_driver.hpp>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/spi_bus.hpp>
#include <vector>

namespace eurorack::drivers::led {

/**
 * @brief Buffered TLC5947 chain exposed as a generic LedBank.
 */
class Tlc5947 final : public LedBank {
  public:
    /**
     * @brief Constructs a driver for one or more cascaded devices.
     * @param spi SPI bus that must outlive the driver.
     * @param latch Latch output that must outlive the driver.
     * @param deviceCount Number of cascaded TLC5947 devices. Each contributes
     * twenty-four logical channels.
     * @param outputEnable Optional active-low blank or OE output.
     */
    Tlc5947(eurorack::io::SpiBus& spi,
            eurorack::io::DigitalOutput& latch,
            std::size_t deviceCount,
            eurorack::io::DigitalOutput* outputEnable = nullptr) noexcept;

    /** @brief Returns twenty-four times the device count. @return Logical channel count. */
    [[nodiscard]] std::size_t channelCount() const noexcept override;

    /**
     * @brief Stores a logical 16-bit channel brightness.
     * @param channel Zero-based channel index.
     * @param brightness Logical brightness. The most significant twelve bits are
     * used in the hardware frame.
     * @return Success or InvalidArgument when the channel is out of range.
     */
    eurorack::io::IoResult setBrightness(std::size_t channel,
                                         std::uint16_t brightness) noexcept override;

    /**
     * @brief Returns one buffered logical brightness.
     * @param channel Zero-based channel index.
     * @return Buffered value, or zero when the channel is out of range.
     */
    [[nodiscard]] std::uint16_t brightness(std::size_t channel) const noexcept override;

    /** @brief Clears all buffered channels. @return Success. */
    eurorack::io::IoResult clear() noexcept override;

    /**
     * @brief Builds the 12-bit PWM frame, transfers it, and pulses latch.
     * @return Success or the SPI transaction error.
     */
    eurorack::io::IoResult flush() noexcept override;

    /**
     * @brief Controls the optional active-low output-enable pin.
     * @param enabled True to enable outputs, false to disable them.
     */
    void setEnabled(bool enabled) noexcept;

  private:
    /**
     * @brief Rebuilds the packed device frame from buffered logical values.
     *
     * @details
     * Channel order is reversed for daisy-chain transmission and each logical
     * 16-bit value is shifted down to the controller's 12-bit PWM resolution.
     */
    void buildFrame() noexcept;

    eurorack::io::SpiBus& spi_;          ///< Non-owning SPI bus reference.
    eurorack::io::DigitalOutput& latch_; ///< Latch output.
    eurorack::io::DigitalOutput* oe_;    ///< Optional active-low OE output.
    std::vector<std::uint16_t> values_;  ///< Logical 16-bit channel values.
    std::vector<std::uint8_t> frame_;    ///< Packed 12-bit PWM frame.
};

} // namespace eurorack::drivers::led
