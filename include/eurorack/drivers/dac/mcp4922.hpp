/**
 * @file include/eurorack/drivers/dac/mcp4922.hpp
 * @brief Declares a buffered driver for the MCP4922 dual 12-bit SPI DAC.
 *
 * @details
 * The driver stores one code, gain setting, and shutdown state per channel and
 * transfers 16-bit command frames through the abstract SpiBus interface. The
 * SPI bus, chip-select output, and optional LDAC output are non-owning
 * dependencies and must outlive the driver.
 *
 * The class operates on raw DAC codes. Reference-voltage selection, output
 * buffering, bipolar conversion, Eurorack output protection, and voltage
 * calibration remain responsibilities of the consuming hardware and firmware.
 * Methods are synchronous and are not internally synchronized.
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

#include <array>
#include <cstdint>
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>

namespace eurorack::drivers::dac {

/**
 * @brief Selects one physical MCP4922 output channel.
 */
enum class Mcp4922Channel : std::uint8_t {
    A, ///< DAC channel A.
    B  ///< DAC channel B.
};

/**
 * @brief Selects the MCP4922 output-amplifier gain bit.
 *
 * @details
 * OneX selects a gain of one. TwoX selects a gain of two. The actual output
 * range also depends on the reference voltage and the external analog circuit.
 */
enum class Mcp4922Gain : std::uint8_t {
    OneX, ///< Output gain of one.
    TwoX  ///< Output gain of two.
};

/**
 * @brief Buffered MCP4922 dual-DAC driver.
 *
 * @details
 * Setter methods update only the driver's local channel state. Call
 * flushChannel() or flushBoth() to transfer that state to the device. When an
 * LDAC output is supplied, flushBoth() writes both input registers before
 * pulsing LDAC, allowing the two outputs to update together. Without LDAC,
 * output timing follows the MCP4922 hardware wiring and command behavior.
 *
 * Construction performs no SPI transfer. It places chip select high and, when
 * present, LDAC high. The caller is responsible for configuring the underlying
 * platform peripherals before the first flush.
 */
class Mcp4922 final {
  public:
    /**
     * @brief Constructs a driver around caller-owned bus and pin adapters.
     *
     * @param spi SPI bus used for all command transfers. It must outlive this
     * object.
     * @param chipSelect Active-low chip-select output. It must outlive this
     * object.
     * @param loadDac Optional active-low LDAC output used for synchronous output
     * updates. Pass nullptr when LDAC is tied permanently active or controlled
     * elsewhere.
     */
    Mcp4922(eurorack::io::SpiBus& spi,
            eurorack::io::DigitalOutput& chipSelect,
            eurorack::io::DigitalOutput* loadDac = nullptr) noexcept;

    /**
     * @brief Stores a 12-bit code for one channel.
     *
     * @details
     * Values above 4095 are clamped to 4095. No bus transaction is performed.
     *
     * @param channel Channel whose buffered code is changed.
     * @param code Raw DAC code in the inclusive range 0 through 4095.
     */
    void setCode(Mcp4922Channel channel, std::uint16_t code) noexcept;

    /**
     * @brief Returns the buffered 12-bit code for one channel.
     *
     * @param channel Channel to inspect.
     * @return Buffered code in the inclusive range 0 through 4095.
     */
    [[nodiscard]] std::uint16_t code(Mcp4922Channel channel) const noexcept;

    /**
     * @brief Stores the gain selection for one channel.
     *
     * @param channel Channel whose gain bit is changed.
     * @param gain Requested hardware gain selection.
     */
    void setGain(Mcp4922Channel channel, Mcp4922Gain gain) noexcept;

    /**
     * @brief Stores the active or shutdown state for one channel.
     *
     * @details
     * Disabled channels are encoded with the MCP4922 shutdown bit cleared. The
     * state reaches the device only after a flush operation.
     *
     * @param channel Channel whose state is changed.
     * @param enabled True for normal operation, false for hardware shutdown.
     */
    void setEnabled(Mcp4922Channel channel, bool enabled) noexcept;

    /**
     * @brief Transfers the buffered state of one channel.
     *
     * @param channel Channel whose command frame is transmitted.
     * @return Success when the complete SPI transaction succeeds; otherwise the
     * error returned by the SPI implementation.
     */
    eurorack::io::IoResult flushChannel(Mcp4922Channel channel) noexcept;

    /**
     * @brief Transfers both buffered channels and requests a common latch.
     *
     * @details
     * Channel A is transferred first and channel B second. If either transfer
     * fails, the method returns immediately and does not report success. When an
     * LDAC pin is available, it is pulsed only after both transfers succeed.
     *
     * @return Success when both SPI transactions and the optional latch action
     * complete; otherwise the first bus error encountered.
     */
    eurorack::io::IoResult flushBoth() noexcept;

    /**
     * @brief Pulses the optional active-low LDAC output.
     *
     * @details
     * The method does nothing when no LDAC output was supplied. It performs no
     * delay; pulse width is determined by two consecutive digital-output calls
     * and the concrete platform adapter.
     */
    void latchOutputs() noexcept;

  private:
    /**
     * @brief Builds the MCP4922 16-bit command frame for one channel.
     * @param channel Channel whose buffered state is encoded.
     * @return Complete command frame in host integer form.
     */
    [[nodiscard]] std::uint16_t buildFrame(Mcp4922Channel channel) const noexcept;

    /**
     * @brief Transfers one already encoded 16-bit command frame.
     * @param frame Command frame to transmit most-significant byte first.
     * @return Success or the error returned by the SPI bus.
     */
    eurorack::io::IoResult writeFrame(std::uint16_t frame) noexcept;

    eurorack::io::SpiBus& spi_;            ///< Non-owning SPI bus reference.
    eurorack::io::DigitalOutput& cs_;      ///< Active-low chip-select output.
    eurorack::io::DigitalOutput* ldac_;    ///< Optional active-low LDAC output.
    std::array<std::uint16_t, 2> codes_{}; ///< Buffered 12-bit channel codes.
    std::array<Mcp4922Gain, 2> gains_{Mcp4922Gain::OneX,
                                      Mcp4922Gain::OneX}; ///< Buffered channel gain settings.
    std::array<bool, 2> enabled_{true, true};             ///< Buffered channel shutdown states.
};

} // namespace eurorack::drivers::dac
