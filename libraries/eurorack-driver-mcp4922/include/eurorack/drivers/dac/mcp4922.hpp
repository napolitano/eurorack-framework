/**
 * @file mcp4922.hpp
 * @brief Declares an allocation-free MCP4922 dual 12-bit SPI DAC driver.
 * @details The caller supplies the SPI bus and control pins. The driver keeps fixed-size channel
 * state only and exposes the reference-buffer bit and SPI clock as configuration.
 * @author Axel Napolitano
 * @date 2026
 * @par License PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#pragma once
#if defined(__AVR__)
#include <eurorack/compat/avr/array.hpp>
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <array>
#include <cstdint>
#endif
#include <eurorack/io/digital_io.hpp>
#include <eurorack/io/io_result.hpp>
#include <eurorack/io/spi_bus.hpp>
namespace eurorack::drivers::dac {
/** @brief Selects a physical DAC channel. */
enum class Mcp4922Channel : std::uint8_t { A, B };
/** @brief Selects the output-amplifier gain. */
enum class Mcp4922Gain : std::uint8_t { OneX, TwoX };
/** @brief Selects whether the voltage-reference input is buffered. */
enum class Mcp4922ReferenceBuffer : std::uint8_t { Unbuffered, Buffered };
/** @brief Static MCP4922 bus and reference configuration. */
struct Mcp4922Config final {
    std::uint32_t spiClockHertz{8'000'000U}; ///< Requested SPI clock.
    Mcp4922ReferenceBuffer referenceBuffer{Mcp4922ReferenceBuffer::Unbuffered}; ///< Reference mode.
};
/** @brief Buffered, allocation-free MCP4922 driver. */
class Mcp4922 final {
  public:
    /**
     * @brief Constructs the driver without starting a bus transaction.
     * @param spi Caller-owned SPI bus.
     * @param chipSelect Caller-owned active-low chip-select output.
     * @param loadDac Optional caller-owned active-low LDAC output.
     * @param config SPI and reference-buffer configuration.
     */
    Mcp4922(eurorack::io::SpiBus& spi,
            eurorack::io::DigitalOutput& chipSelect,
            eurorack::io::DigitalOutput* loadDac = nullptr,
            Mcp4922Config config = {}) noexcept;
    /** @brief Stores a clamped 12-bit code. @param channel Target channel. @param code Raw code. */
    void setCode(Mcp4922Channel channel, std::uint16_t code) noexcept;
    /** @brief Returns a buffered code. @param channel Target channel. @return Code from 0 to 4095.
     */
    [[nodiscard]] std::uint16_t code(Mcp4922Channel channel) const noexcept;
    /** @brief Changes the buffered gain bit. @param channel Target channel. @param gain Gain mode.
     */
    void setGain(Mcp4922Channel channel, Mcp4922Gain gain) noexcept;
    /** @brief Changes the buffered shutdown bit. @param channel Target channel. @param enabled True
     * for active output. */
    void setEnabled(Mcp4922Channel channel, bool enabled) noexcept;
    /** @brief Changes the reference-buffer bit for subsequent frames. @param mode Reference mode.
     */
    void setReferenceBuffer(Mcp4922ReferenceBuffer mode) noexcept;
    /** @brief Returns the configured reference mode. @return Current reference mode. */
    [[nodiscard]] Mcp4922ReferenceBuffer referenceBuffer() const noexcept;
    /** @brief Transfers one channel frame. @param channel Target channel. @return Bus operation
     * status. */
    eurorack::io::IoResult flushChannel(Mcp4922Channel channel) noexcept;
    /** @brief Transfers both channels and optionally pulses LDAC. @return First failure or Success.
     */
    eurorack::io::IoResult flushBoth() noexcept;
    /** @brief Pulses the optional LDAC output. */
    void latchOutputs() noexcept;

  private:
    /** @brief Encodes one command frame. @param channel Target channel. @return Encoded 16-bit
     * frame. */
    [[nodiscard]] std::uint16_t buildFrame(Mcp4922Channel channel) const noexcept;
    /** @brief Transfers one encoded frame. @param frame Encoded frame. @return Bus operation
     * status. */
    eurorack::io::IoResult writeFrame(std::uint16_t frame) noexcept;
    eurorack::io::SpiBus& spi_;            ///< Non-owning SPI bus reference.
    eurorack::io::DigitalOutput& cs_;      ///< Active-low chip-select output.
    eurorack::io::DigitalOutput* ldac_;    ///< Optional active-low LDAC output.
    Mcp4922Config config_{};               ///< SPI clock and reference-buffer configuration.
    std::array<std::uint16_t, 2> codes_{}; ///< Buffered 12-bit channel codes.
    std::array<Mcp4922Gain, 2> gains_{Mcp4922Gain::OneX,
                                      Mcp4922Gain::OneX}; ///< Buffered channel gain settings.
    std::array<bool, 2> enabled_{true, true};             ///< Buffered channel shutdown states.
};
} // namespace eurorack::drivers::dac
