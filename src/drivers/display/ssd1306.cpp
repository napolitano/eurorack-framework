/**
 * @file src/drivers/display/ssd1306.cpp
 * @brief Implements the SSD1306 monochrome OLED driver.
 *
 * @details
 * Uses horizontal addressing mode and converts the row-major canvas into SSD1306 page layout.
 *
 * The framework canvas packs pixels horizontally, while SSD1306 display RAM
 * packs eight vertical pixels into each byte. flush() transposes one page at a
 * time into a fixed 128-byte scratch buffer and sends bounded I2C chunks.
 *
 * No dynamic allocation occurs. A successful call means that the bus accepted
 * every command and data packet; it does not verify optical panel output.
 *
 * The framework canvas packs pixels horizontally, while SSD1306 display RAM
 * packs eight vertical pixels into each byte. flush() transposes one page at a
 * time into a fixed 128-byte scratch buffer. Data is then split into bounded
 * I2C chunks so the driver does not depend on a particular Wire buffer size.
 *
 * The implementation performs no dynamic allocation. A successful call means
 * that the bus accepted all bytes; it cannot confirm that the panel displayed
 * them physically.
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
#include <eurorack/drivers/display/ssd1306.hpp>

namespace eurorack::drivers::display {

namespace {

constexpr std::uint8_t CONTROL_COMMAND = 0x00U;
constexpr std::uint8_t CONTROL_DATA = 0x40U;
constexpr std::size_t MAX_I2C_PAYLOAD = 16U;
constexpr std::size_t MAX_PANEL_WIDTH = 128U;
constexpr std::size_t MAX_COMMAND_COUNT = 31U;

} // namespace

Ssd1306::Ssd1306(eurorack::io::I2cBus& bus, const Ssd1306Config config) noexcept
    : bus_(bus), config_(config) {}

eurorack::io::IoResult Ssd1306::initialize() noexcept {
    auto result = bus_.setClock(config_.i2cClockHz);
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    const std::uint8_t panelHeight = static_cast<std::uint8_t>(config_.height);
    const std::uint8_t multiplex = static_cast<std::uint8_t>(panelHeight - 1U);
    const std::uint8_t comPins = panelHeight == 64U ? 0x12U : 0x02U;
    const std::uint8_t chargePump =
        static_cast<std::uint8_t>(config_.chargePumpEnabled ? 0x14U : 0x10U);
    const std::uint8_t segmentRemap =
        static_cast<std::uint8_t>(config_.segmentRemap ? 0xA1U : 0xA0U);
    const std::uint8_t comScan = static_cast<std::uint8_t>(config_.comScanRemapped ? 0xC8U : 0xC0U);
    const std::uint8_t precharge = config_.chargePumpEnabled ? 0xF1U : 0x22U;

    const std::array<std::uint8_t, 25> commands{
        0xAEU,        0xD5U,     0x80U, 0xA8U,      multiplex, 0xD3U,
        0x00U,        0x40U,     0x8DU, chargePump, 0x20U,     0x00U,
        segmentRemap, comScan,   0xDAU, comPins,    0x81U,     config_.contrast,
        0xD9U,        precharge, 0xDBU, 0x40U,      0xA4U,     0xA6U};

    result = writeCommands(commands.data(), commands.size());
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    result = configureAddressWindow();
    if (result != eurorack::io::IoResult::Success) {
        return result;
    }

    result = writeCommand(0xAFU);
    initialized_ = result == eurorack::io::IoResult::Success;
    return result;
}

eurorack::io::IoResult Ssd1306::flush(const eurorack::display::MonochromeCanvas& canvas) noexcept {
    if (!initialized_) {
        return eurorack::io::IoResult::Busy;
    }

    if (config_.width == 0U || config_.width > MAX_PANEL_WIDTH ||
        canvas.width() != static_cast<std::int32_t>(config_.width) ||
        canvas.height() != static_cast<std::int32_t>(height())) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    std::array<std::uint8_t, MAX_PANEL_WIDTH> pageBuffer{};
    const std::uint8_t pageCount = static_cast<std::uint8_t>(height() / 8U);

    for (std::uint8_t page = 0U; page < pageCount; ++page) {
        std::fill(pageBuffer.begin(), pageBuffer.end(), 0U);

        for (std::uint16_t x = 0U; x < config_.width; ++x) {
            // Transpose the canvas' pixel addressing into the controller's
            // least-significant-bit-at-top page byte.
            std::uint8_t value = 0U;

            for (std::uint8_t bit = 0U; bit < 8U; ++bit) {
                const std::int32_t y =
                    static_cast<std::int32_t>(static_cast<std::uint16_t>(page) * 8U + bit);

                if (canvas.pixel({static_cast<std::int32_t>(x), y})) {
                    value = static_cast<std::uint8_t>(value | static_cast<std::uint8_t>(1U << bit));
                }
            }

            pageBuffer[x] = value;
        }

        const std::array<std::uint8_t, 6> window{
            0x21U, 0x00U, static_cast<std::uint8_t>(config_.width - 1U), 0x22U, page, page};

        auto result = writeCommands(window.data(), window.size());
        if (result != eurorack::io::IoResult::Success) {
            return result;
        }

        result = writeData(pageBuffer.data(), config_.width);
        if (result != eurorack::io::IoResult::Success) {
            return result;
        }
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult Ssd1306::setContrast(const std::uint8_t contrast) noexcept {
    const std::array<std::uint8_t, 2> commands{0x81U, contrast};

    const auto result = writeCommands(commands.data(), commands.size());

    if (result == eurorack::io::IoResult::Success) {
        config_.contrast = contrast;
    }

    return result;
}

eurorack::io::IoResult Ssd1306::setInverted(const bool inverted) noexcept {
    return writeCommand(static_cast<std::uint8_t>(inverted ? 0xA7U : 0xA6U));
}

eurorack::io::IoResult Ssd1306::setDisplayEnabled(const bool enabled) noexcept {
    return writeCommand(static_cast<std::uint8_t>(enabled ? 0xAFU : 0xAEU));
}

std::uint16_t Ssd1306::width() const noexcept {
    return config_.width;
}

std::uint16_t Ssd1306::height() const noexcept {
    return static_cast<std::uint16_t>(config_.height);
}

eurorack::io::IoResult Ssd1306::writeCommand(const std::uint8_t command) noexcept {
    return writeCommands(&command, 1U);
}

eurorack::io::IoResult Ssd1306::writeCommands(const std::uint8_t* const commands,
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

eurorack::io::IoResult Ssd1306::writeData(const std::uint8_t* const data,
                                          const std::size_t size) noexcept {
    if (data == nullptr && size > 0U) {
        return eurorack::io::IoResult::InvalidArgument;
    }

    std::array<std::uint8_t, MAX_I2C_PAYLOAD + 1U> packet{};
    packet[0] = CONTROL_DATA;

    std::size_t offset = 0U;

    // Reserve byte zero for the SSD1306 control byte. Limiting payload
    // size avoids depending on a particular platform Wire buffer capacity.
    while (offset < size) {
        const std::size_t chunk = std::min(MAX_I2C_PAYLOAD, size - offset);

        std::copy(data + offset, data + offset + chunk, packet.begin() + 1);

        const auto result = bus_.write(config_.address, packet.data(), chunk + 1U);

        if (result != eurorack::io::IoResult::Success) {
            return result;
        }

        offset += chunk;
    }

    return eurorack::io::IoResult::Success;
}

eurorack::io::IoResult Ssd1306::configureAddressWindow() noexcept {
    const std::uint8_t pages = static_cast<std::uint8_t>(height() / 8U);

    const std::array<std::uint8_t, 6> commands{0x21U,
                                               0x00U,
                                               static_cast<std::uint8_t>(config_.width - 1U),
                                               0x22U,
                                               0x00U,
                                               static_cast<std::uint8_t>(pages - 1U)};

    return writeCommands(commands.data(), commands.size());
}

} // namespace eurorack::drivers::display
