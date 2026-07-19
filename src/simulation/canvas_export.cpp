/**
 * @file src/simulation/canvas_export.cpp
 * @brief Implements PBM and ASCII canvas export.
 *
 * @details
 * Exports monochrome canvases into human-inspectable native artifacts.
 * ASCII output favors compact test diagnostics, while PBM output produces a standards-based image that can be opened by common tools. Export reads pixels through the public canvas API so clipping and storage layout remain encapsulated. File output is native-only and may throw internally; public functions translate failures into operation results where applicable.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#if !defined(ARDUINO)

#include <eurorack/simulation/canvas_export.hpp>
#include <fstream>
#include <sstream>

namespace eurorack::simulation {

eurorack::io::IoResult exportCanvasPbm(const eurorack::display::MonochromeCanvas& canvas,
                                       const std::string& path) noexcept {
    try {
        std::ofstream file(path, std::ios::binary);
        if (!file) {
            return eurorack::io::IoResult::DataError;
        }

        file << "P4\n" << canvas.width() << ' ' << canvas.height() << '\n';

        const std::size_t stride = static_cast<std::size_t>((canvas.width() + 7) / 8);

        for (std::int32_t y = 0; y < canvas.height(); ++y) {
            for (std::size_t byte = 0U; byte < stride; ++byte) {
                std::uint8_t value = 0U;

                for (std::uint8_t bit = 0U; bit < 8U; ++bit) {
                    const std::int32_t x = static_cast<std::int32_t>(byte * 8U + bit);

                    if (x < canvas.width() && canvas.pixel({x, y})) {
                        value = static_cast<std::uint8_t>(value |
                                                          static_cast<std::uint8_t>(0x80U >> bit));
                    }
                }

                file.put(static_cast<char>(value));
            }
        }

        return file ? eurorack::io::IoResult::Success : eurorack::io::IoResult::DataError;
    } catch (...) {
        return eurorack::io::IoResult::UnknownError;
    }
}

std::string canvasToAscii(const eurorack::display::MonochromeCanvas& canvas,
                          const char offCharacter,
                          const char onCharacter) {
    std::ostringstream output;

    for (std::int32_t y = 0; y < canvas.height(); ++y) {
        for (std::int32_t x = 0; x < canvas.width(); ++x) {
            output << (canvas.pixel({x, y}) ? onCharacter : offCharacter);
        }

        output << '\n';
    }

    return output.str();
}

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
