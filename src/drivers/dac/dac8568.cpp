/**
 * @file src/drivers/dac/dac8568.cpp
 * @brief Implements the DAC8568 eight-channel SPI DAC driver.
 *
 * @details
 * Builds exact 32-bit DAC8568 serial frames and maintains one buffered 16-bit code per output channel.
 * Normal data writes use the command, address, data, and feature fields. Device-control commands such as power mode, reset, clear-code selection, and internal-reference control use command-specific bit layouts and are therefore packed explicitly. Chip select is asserted only after beginTransaction succeeds and is released before ending the transaction.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <eurorack/drivers/dac/dac8568.hpp>

namespace eurorack::drivers::dac {

Dac8568::Dac8568(eurorack::io::SpiBus& spi, eurorack::io::DigitalOutput& chipSelect) noexcept
    : spi_(spi), chipSelect_(chipSelect) {
    chipSelect_.writeHigh(true);
}

void Dac8568::setCode(const Dac8568Channel channel, const std::uint16_t codeValue) noexcept {
    if (channel != Dac8568Channel::All) {
        codes_[channelIndex(channel)] = codeValue;
    }
}

std::uint16_t Dac8568::code(const Dac8568Channel channel) const noexcept {
    return channel == Dac8568Channel::All ? 0U : codes_[channelIndex(channel)];
}

eurorack::io::IoResult Dac8568::writeInput(const Dac8568Channel channel) noexcept {
    if (channel == Dac8568Channel::All) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    return transferFrame(Command::WriteInput, static_cast<std::uint8_t>(channel), code(channel));
}

eurorack::io::IoResult Dac8568::writeAndUpdate(const Dac8568Channel channel) noexcept {
    if (channel == Dac8568Channel::All) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    return transferFrame(
        Command::WriteInputUpdateChannel, static_cast<std::uint8_t>(channel), code(channel));
}

eurorack::io::IoResult Dac8568::writeAndUpdateAll(const Dac8568Channel channel) noexcept {
    if (channel == Dac8568Channel::All) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    return transferFrame(
        Command::WriteInputUpdateAll, static_cast<std::uint8_t>(channel), code(channel));
}

eurorack::io::IoResult Dac8568::flushAll() noexcept {
    for (std::uint8_t index = 0U; index < 8U; ++index) {
        const auto channel = static_cast<Dac8568Channel>(index);
        const auto result = index == 7U ? writeAndUpdateAll(channel) : writeInput(channel);

        if (result != eurorack::io::IoResult::Success) {
            return result;
        }
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult Dac8568::setPowerMode(const std::uint8_t channelMask,
                                             const Dac8568PowerMode mode) noexcept {
    const std::uint32_t frame =
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(Command::Power)) << 24U) |
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(mode) & 0x03U) << 8U) |
        static_cast<std::uint32_t>(channelMask);

    return transferRawFrame(frame);
}

eurorack::io::IoResult Dac8568::setInternalReference(const bool enabled) noexcept {
    return transferFrame(Command::InternalReference, 0U, 0U, enabled ? 1U : 0U);
}

eurorack::io::IoResult Dac8568::reset() noexcept {
    return transferRawFrame(static_cast<std::uint32_t>(static_cast<std::uint8_t>(Command::Reset))
                            << 24U);
}

eurorack::io::IoResult Dac8568::setClearCode(const Dac8568ClearCode clearCode) noexcept {
    return transferFrame(Command::Clear, 0U, 0U, static_cast<std::uint8_t>(clearCode));
}

std::size_t Dac8568::channelIndex(const Dac8568Channel channel) noexcept {
    return static_cast<std::size_t>(static_cast<std::uint8_t>(channel) & 0x07U);
}

eurorack::io::IoResult Dac8568::transferFrame(const Command command,
                                              const std::uint8_t address,
                                              const std::uint16_t data,
                                              const std::uint8_t feature) noexcept {
    const std::uint32_t frame =
        (static_cast<std::uint32_t>(static_cast<std::uint8_t>(command) & 0x0FU) << 24U) |
        (static_cast<std::uint32_t>(address & 0x0FU) << 20U) |
        (static_cast<std::uint32_t>(data) << 4U) | static_cast<std::uint32_t>(feature & 0x0FU);

    return transferRawFrame(frame);
}

eurorack::io::IoResult Dac8568::transferRawFrame(const std::uint32_t frame) noexcept {
    const std::uint8_t bytes[4]{static_cast<std::uint8_t>((frame >> 24U) & 0xFFU),
                                static_cast<std::uint8_t>((frame >> 16U) & 0xFFU),
                                static_cast<std::uint8_t>((frame >> 8U) & 0xFFU),
                                static_cast<std::uint8_t>(frame & 0xFFU)};

    const eurorack::io::SpiSettings settings{50'000'000U,
                                             eurorack::io::SpiMode::Mode1,
                                             eurorack::io::SpiBitOrder::MostSignificantBitFirst};

    const auto beginResult = spi_.beginTransaction(settings);
    if (beginResult != eurorack::io::IoResult::Success) {
        return beginResult;
    }

    chipSelect_.writeHigh(false);
    const auto transferResult = spi_.transfer(bytes, nullptr, sizeof(bytes));
    chipSelect_.writeHigh(true);
    spi_.endTransaction();

    return transferResult;
}

} // namespace eurorack::drivers::dac
