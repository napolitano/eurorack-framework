/**
 * @file include/eurorack/io/analog_io.hpp
 * @brief Declares raw analog input and output channel interfaces.
 *
 * @details
 * Drivers expose integer converter codes. Voltage conversion and calibration remain separate
 * framework layers.
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

#if defined(__AVR__)
#include <eurorack/compat/avr/cstdint.hpp>
#else
#include <cstdint>
#endif
#include <eurorack/io/io_result.hpp>

namespace eurorack::io {

/**
 * @brief Raw sample returned by an analog input channel.
 */
struct AnalogSample final {
    std::uint32_t code{0U};             ///< Raw converter code.
    IoResult result{IoResult::Success}; ///< Acquisition status.
};

/**
 * @brief Read-only raw analog input channel.
 */
class AnalogInputChannel {
  public:
    /**
     * @brief Destroys an analog-input backend through its interface.
     */
    virtual ~AnalogInputChannel() = default;

    /**
     * @brief Reads one raw converter sample.
     *
     * @return Raw code and acquisition status.
     */
    [[nodiscard]] virtual AnalogSample readRaw() noexcept = 0;

    /**
     * @brief Returns the largest valid converter code.
     *
     * @return Inclusive maximum code, such as 1023 for a 10-bit ADC.
     */
    [[nodiscard]] virtual std::uint32_t maximumCode() const noexcept = 0;
};

/**
 * @brief Writable raw analog output channel.
 */
class AnalogOutputChannel {
  public:
    /**
     * @brief Destroys an analog-output backend through its interface.
     */
    virtual ~AnalogOutputChannel() = default;

    /**
     * @brief Writes one raw converter code.
     *
     * @param code Requested converter code.
     * @return Operation status. InvalidArgument indicates a code above maximumCode().
     */
    virtual IoResult writeRaw(std::uint32_t code) noexcept = 0;

    /**
     * @brief Returns the largest valid converter code.
     *
     * @return Inclusive maximum code supported by the output converter.
     */
    [[nodiscard]] virtual std::uint32_t maximumCode() const noexcept = 0;

    /**
     * @brief Returns the most recently accepted raw output code.
     *
     * @return Last code successfully accepted by writeRaw().
     */
    [[nodiscard]] virtual std::uint32_t lastWrittenCode() const noexcept = 0;
};

} // namespace eurorack::io
