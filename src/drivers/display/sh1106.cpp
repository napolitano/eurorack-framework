/**
 * @file src/drivers/display/sh1106.cpp
 * @brief Implements the SH1106 monochrome OLED driver.
 *
 * @details
 * Converts the row-major framework canvas into SH1106 page data and applies the configurable column
 * offset.
 *
 * Common 128-pixel modules expose 132 columns of controller RAM, so the visible
 * image often begins at column two. The configurable offset makes that hardware
 * detail explicit instead of hiding it in drawing coordinates.
 *
 * Conversion uses a fixed 128-byte page buffer and bounded I2C packets. No
 * dynamic allocation occurs in initialize() or flush().
 *
 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 *
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#include <algorithm>
#include <array>
#include <eurorack/drivers/display/sh1106.hpp>

namespace eurorack::drivers::display {

namespace {
constexpr std::uint8_t CONTROL_COMMAND = 0x00U;
constexpr std::uint8_t CONTROL_DATA = 0x40U;
constexpr std::size_t MAX_DATA_CHUNK = 16U;
constexpr std::size_t MAX_PANEL_WIDTH = 128U;
constexpr std::size_t MAX_COMMAND_COUNT = 31U;
} // namespace

Sh1106::Sh1106(eurorack::io::I2cBus& bus, const Sh1106Config config) noexcept
    : bus_(bus), config_(config) {}

eurorack::io::IoResult Sh1106::initialize() noexcept {
    auto result = bus_.setClock(config_.i2cClockHz);
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    const std::uint8_t multiplex = static_cast<std::uint8_t>(config_.height - 1U);
    const std::uint8_t segment = static_cast<std::uint8_t>(config_.segmentRemap ? 0xA1U : 0xA0U);
    const std::uint8_t comScan = static_cast<std::uint8_t>(config_.comScanRemapped ? 0xC8U : 0xC0U);
    const std::uint8_t pump = static_cast<std::uint8_t>(config_.chargePumpEnabled ? 0x8BU : 0x8AU);

    const std::array<std::uint8_t, 23> commands{
        0xAEU, 0xD5U, 0x80U,   0xA8U,   multiplex, 0xD3U, 0x00U, 0x40U,
        0xADU, pump,  segment, comScan, 0xDAU,     0x12U, 0x81U, config_.contrast,
        0xD9U, 0x22U, 0xDBU,   0x35U,   0xA4U,     0xA6U, 0xAFU};

    result = writeCommands(commands.data(), commands.size());
    initialized_ = result == eurorack::io::IoResult::Success;
    return result;
}

eurorack::io::IoResult Sh1106::flush(const eurorack::display::MonochromeCanvas& canvas) noexcept {
    if (!initialized_) {
        return eurorack::io::IoResult::Busy;
    }

    if (canvas.width() != static_cast<std::int32_t>(config_.width) ||
        canvas.height() != static_cast<std::int32_t>(config_.height) || config_.height == 0U ||
        config_.height % 8U != 0U) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (config_.width > MAX_PANEL_WIDTH) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    std::array<std::uint8_t, MAX_PANEL_WIDTH> page{};

    const std::uint16_t pageCount = static_cast<std::uint16_t>(config_.height / 8U);

    // Convert and transfer one eight-pixel-high page at a time. This keeps
    // memory use independent of display height.
    for (std::uint16_t pageIndex = 0U; pageIndex < pageCount; ++pageIndex) {
        std::fill(page.begin(), page.end(), 0U);

        for (std::uint16_t x = 0U; x < config_.width; ++x) {
            std::uint8_t value = 0U;

            for (std::uint8_t bit = 0U; bit < 8U; ++bit) {
                const std::int32_t y = static_cast<std::int32_t>(pageIndex * 8U + bit);

                if (canvas.pixel({static_cast<std::int32_t>(x), y})) {
                    value = static_cast<std::uint8_t>(value | static_cast<std::uint8_t>(1U << bit));
                }
            }

            page[x] = value;
        }

        // SH1106 splits the start column across low- and high-nibble
        // commands. The offset aligns 128 visible columns within 132 RAM columns.        // Split
        // the visible start column into the controller's low- and high-nibble address commands.

        const std::uint8_t column = config_.columnOffset;
        const std::array<std::uint8_t, 3> commands{
            static_cast<std::uint8_t>(0xB0U | static_cast<std::uint8_t>(pageIndex)),
            static_cast<std::uint8_t>(column & 0x0FU),
            static_cast<std::uint8_t>(0x10U | ((column >> 4U) & 0x0FU))};

        auto result = writeCommands(commands.data(), commands.size());
        if (result != eurorack::io::IoResult::Success) {
            return result;
        }

        result = writeData(page.data(), config_.width);
        if (result != eurorack::io::IoResult::Success) {
            return result;
        }
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult Sh1106::setContrast(const std::uint8_t contrast) noexcept {
    const std::array<std::uint8_t, 2> commands{0x81U, contrast};
    const auto result = writeCommands(commands.data(), commands.size());

    if (result == eurorack::io::IoResult::Success) {
        config_.contrast = contrast;
    }

    return result;
}

eurorack::io::IoResult Sh1106::setInverted(const bool inverted) noexcept {
    return writeCommand(static_cast<std::uint8_t>(inverted ? 0xA7U : 0xA6U));
}

eurorack::io::IoResult Sh1106::setDisplayEnabled(const bool enabled) noexcept {
    return writeCommand(static_cast<std::uint8_t>(enabled ? 0xAFU : 0xAEU));
}

eurorack::io::IoResult Sh1106::writeCommand(const std::uint8_t command) noexcept {
    return writeCommands(&command, 1U);
}

eurorack::io::IoResult Sh1106::writeCommands(const std::uint8_t* const commands,
                                             const std::size_t size) noexcept {
    if (commands == nullptr && size > 0U) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    if (size > MAX_COMMAND_COUNT) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    std::array<std::uint8_t, MAX_COMMAND_COUNT + 1U> packet{};
    packet[0] = CONTROL_COMMAND;
    if (size > 0U) {
        std::copy(commands, commands + size, packet.begin() + 1);
    }

    return bus_.write(config_.address, packet.data(), size + 1U);
}

eurorack::io::IoResult Sh1106::writeData(const std::uint8_t* const data,
                                         const std::size_t size) noexcept {
    if (data == nullptr && size > 0U) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    std::array<std::uint8_t, MAX_DATA_CHUNK + 1U> packet{};
    packet[0] = CONTROL_DATA;

    std::size_t offset = 0U;
    while (offset < size) {
        const std::size_t chunk = std::min(MAX_DATA_CHUNK, size - offset);
        std::copy(data + offset, data + offset + chunk, packet.begin() + 1);

        const auto result = bus_.write(config_.address, packet.data(), chunk + 1U);

        if (result != eurorack::io::IoResult::Success) {
            return result;
        }

        offset += chunk;
    }

    return eurorack::io::IoResult::Success;
}

} // namespace eurorack::drivers::display
