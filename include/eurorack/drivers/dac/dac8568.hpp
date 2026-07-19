/**
 * @file include/eurorack/drivers/dac/dac8568.hpp
 * @brief Declares a DAC8568 eight-channel 16-bit SPI DAC driver.
 *
 * @details
 * Provides buffered channel codes, synchronous update commands, power control, reset, clear, and
 * reference control.
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
 * @ingroup drivers
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>

namespace eurorack::drivers::dac {

/** @brief DAC8568 output channel. */
enum class Dac8568Channel : std::uint8_t {
    A = 0U,
    B = 1U,
    C = 2U,
    D = 3U,
    E = 4U,
    F = 5U,
    G = 6U,
    H = 7U,
    All = 15U
};

/** @brief DAC8568 output power mode. */
enum class Dac8568PowerMode : std::uint8_t {
    Normal = 0U,
    PowerDown1K = 1U,
    PowerDown100K = 2U,
    PowerDownHighImpedance = 3U
};

/**
 * @brief Code loaded by the external CLR pin.
 */
enum class Dac8568ClearCode : std::uint8_t {
    ZeroScale = 0U,
    Midscale = 1U,
    FullScale = 2U,
    IgnoreClearPin = 3U
};

/** @brief DAC8568 eight-channel SPI driver. */
class Dac8568 final {
  public:
    /**
     * @brief Constructs a DAC8568 driver.
     * @param spi SPI bus.
     * @param chipSelect Active-low SYNC pin.
     */
    Dac8568(eurorack::io::SpiBus& spi, eurorack::io::DigitalOutput& chipSelect) noexcept;

    /**
     * @brief Stores one buffered channel code.
     * @param channel Output channel A through H.
     * @param code Full 16-bit DAC code.
     */
    void setCode(Dac8568Channel channel, std::uint16_t code) noexcept;

    /**
     * @brief Returns one buffered channel code.
     * @param channel Output channel A through H.
     * @return Buffered code or zero for All.
     */
    [[nodiscard]] std::uint16_t code(Dac8568Channel channel) const noexcept;

    /** @brief Writes an input register. @param channel Output channel. @return Success when the
     * requested operation completes; otherwise a specific validation, bus, or storage error. */
    [[nodiscard]] eurorack::io::IoResult writeInput(Dac8568Channel channel) noexcept;

    /** @brief Writes and updates one channel. @param channel Output channel. @return Operation
     * result. */
    [[nodiscard]] eurorack::io::IoResult writeAndUpdate(Dac8568Channel channel) noexcept;

    /** @brief Writes one input and updates all outputs. @param channel Input register. @return
     * Success when the SPI transaction completes; otherwise the first bus or state error. */
    [[nodiscard]] eurorack::io::IoResult writeAndUpdateAll(Dac8568Channel channel) noexcept;

    /** @brief Writes all buffered channels and updates together. @return Success when the requested
     * operation completes; otherwise a specific validation, bus, or storage error. */
    [[nodiscard]] eurorack::io::IoResult flushAll() noexcept;

    /**
     * @brief Configures power mode for a channel mask.
     * @param channelMask Bit zero selects channel A.
     * @param mode Requested power mode.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    [[nodiscard]] eurorack::io::IoResult setPowerMode(std::uint8_t channelMask,
                                                      Dac8568PowerMode mode) noexcept;

    /** @brief Enables or disables the internal reference. @param enabled Desired state. @return
     * Success when the SPI transaction completes; otherwise the first bus or state error. */
    [[nodiscard]] eurorack::io::IoResult setInternalReference(bool enabled) noexcept;

    /** @brief Performs a software reset. @return Success when the requested operation completes;
     * otherwise a specific validation, bus, or storage error. */
    [[nodiscard]] eurorack::io::IoResult reset() noexcept;

    /**
     * @brief Configures the code applied by the external CLR pin.
     *
     * @param clearCode Requested clear-code behavior.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    [[nodiscard]] eurorack::io::IoResult setClearCode(Dac8568ClearCode clearCode) noexcept;

  private:
    /**
     * @brief Four-bit command selector encoded in the DAC8568 serial frame.
     *
     * @details
     * This private enumeration mirrors the controller command field. Some
     * commands use the normal address/data/feature layout, while power and reset
     * commands require special packing in the implementation.
     */
    enum class Command : std::uint8_t {
        WriteInput = 0x0U,
        UpdateDac = 0x1U,
        WriteInputUpdateAll = 0x2U,
        WriteInputUpdateChannel = 0x3U,
        Power = 0x4U,
        Clear = 0x5U,
        LdacMask = 0x6U,
        Reset = 0x7U,
        InternalReference = 0x8U
    };

    /** @brief Converts a channel to array index. @param channel Channel. @return Index. */
    [[nodiscard]] static std::size_t channelIndex(Dac8568Channel channel) noexcept;

    /**
     * @brief Transfers one 32-bit command frame.
     * @param command Four-bit command.
     * @param address Four-bit address.
     * @param data Sixteen-bit data field.
     * @param feature Four-bit feature field.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    [[nodiscard]] eurorack::io::IoResult transferFrame(Command command,
                                                       std::uint8_t address,
                                                       std::uint16_t data,
                                                       std::uint8_t feature = 0U) noexcept;

    /**
     * @brief Transfers one already packed 32-bit frame.
     *
     * @param frame Complete DAC8568 serial frame.
     * @return Success when the requested operation completes; otherwise a specific validation, bus,
     * or storage error.
     */
    [[nodiscard]] eurorack::io::IoResult transferRawFrame(std::uint32_t frame) noexcept;

    eurorack::io::SpiBus& spi_;
    eurorack::io::DigitalOutput& chipSelect_;
    std::array<std::uint16_t, 8> codes_{};
};

} // namespace eurorack::drivers::dac
