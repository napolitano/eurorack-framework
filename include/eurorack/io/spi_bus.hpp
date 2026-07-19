/**
 * @file include/eurorack/io/spi_bus.hpp
 * @brief Declares a platform-independent SPI bus interface.
 *
 * @details
 * The bus transfers byte buffers using explicit clock, mode, and bit-order settings. Chip-select
 * remains a separate DigitalOutput.
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 *
 * This file is part of Eurorack Framework. Noncommercial use, modification,
 * and redistribution are permitted under the terms in the repository-root
 * LICENSE file. The separate ADDITIONAL_PERMISSION.md file permits limited
 * five-unit, cost-recovery distribution under its stated conditions.
 * Commercial use requires prior written permission.
 *
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 *
 * @ingroup io
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <eurorack/io/io_result.hpp>

namespace eurorack::io {

/**
 * @brief SPI clock polarity and phase mode.
 */
enum class SpiMode : std::uint8_t {
    Mode0, ///< CPOL 0, CPHA 0.
    Mode1, ///< CPOL 0, CPHA 1.
    Mode2, ///< CPOL 1, CPHA 0.
    Mode3  ///< CPOL 1, CPHA 1.
};

/**
 * @brief SPI bit transmission order.
 */
enum class SpiBitOrder : std::uint8_t {
    MostSignificantBitFirst, ///< Transmit bit 7 first.
    LeastSignificantBitFirst ///< Transmit bit 0 first.
};

/**
 * @brief Settings applied to one SPI transaction.
 */
struct SpiSettings final {
    std::uint32_t clockHertz{1'000'000U}; ///< Maximum requested clock frequency.
    SpiMode mode{SpiMode::Mode0};         ///< Clock polarity and phase mode.
    SpiBitOrder bitOrder{SpiBitOrder::MostSignificantBitFirst}; ///< Bit order.
};

/**
 * @brief Full-duplex SPI bus transaction interface.
 */
class SpiBus {
  public:
    /**
     * @brief Destroys an SPI backend through its interface.
     */
    virtual ~SpiBus() = default;

    /**
     * @brief Starts a transaction using the supplied bus settings.
     *
     * @param settings Clock, mode, and bit-order settings.
     * @return Operation status.
     *
     * Successful beginTransaction() calls must be paired with endTransaction().
     */
    virtual IoResult beginTransaction(const SpiSettings& settings) noexcept = 0;

    /**
     * @brief Transfers a byte buffer in full duplex.
     *
     * @param transmit Data to transmit, or nullptr to transmit backend-defined filler bytes.
     * @param receive Destination for received data, or nullptr to discard received bytes.
     * @param size Number of bytes to transfer.
     * @return Operation status.
     *
     * At least one of transmit or receive must be non-null when size is nonzero.
     */
    virtual IoResult
    transfer(const std::uint8_t* transmit, std::uint8_t* receive, std::size_t size) noexcept = 0;

    /**
     * @brief Ends the current SPI transaction.
     *
     * The backend restores or releases bus state as required by the platform.
     */
    virtual void endTransaction() noexcept = 0;
};

} // namespace eurorack::io
