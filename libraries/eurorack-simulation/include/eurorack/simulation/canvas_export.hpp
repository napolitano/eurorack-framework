/**
 * @file include/eurorack/simulation/canvas_export.hpp
 * @brief Declares portable monochrome canvas export.
 *
 * @details
 * Exports PBM images and human-readable ASCII previews without GUI dependencies.
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
 * @ingroup simulation
 */

#if !defined(ARDUINO)

#pragma once

#include <eurorack/display/monochrome_canvas.hpp>
#include <eurorack/io/io_result.hpp>
#include <string>

namespace eurorack::simulation {

/**
 * @brief Exports the canvas as a binary PBM image.
 *
 * @details
 * The operation is synchronous and writes the complete image to the supplied
 * filesystem path. It does not retain references to the canvas or path after
 * returning.
 *
 * @param canvas Source canvas. The caller retains ownership of the canvas and
 * its backing buffer.
 * @param path Destination filesystem path.
 *
 * @return `IoResult::Success` when the file was opened and written completely;
 * otherwise an appropriate I/O failure result.
 */
[[nodiscard]] eurorack::io::IoResult
exportCanvasPbm(const eurorack::display::MonochromeCanvas& canvas,
                const std::string& path) noexcept;

/**
 * @brief Converts the canvas to a line-oriented ASCII representation.
 *
 * @details
 * The operation is synchronous and does not retain pointers supplied only as
 * call arguments. Ownership, allocation, clipping, and error semantics follow
 * the contract documented for the enclosing type.
 *
 * @param canvas Destination canvas. The caller retains ownership of the canvas and its buffer.
 *
 * @param offCharacter Character emitted for a cleared canvas pixel.
 *
 * @param onCharacter Character emitted for a set canvas pixel.
 *
 * @return An owning string containing the complete representation.
 */
[[nodiscard]] std::string canvasToAscii(const eurorack::display::MonochromeCanvas& canvas,
                                        char offCharacter = ' ',
                                        char onCharacter = '#');

} // namespace eurorack::simulation

#endif // !defined(ARDUINO)
