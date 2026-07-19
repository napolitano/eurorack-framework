/**
 * @file include/eurorack/io/digital_io.hpp
 * @brief Declares platform-independent digital input and output interfaces.
 *
 * @details
 * The interfaces represent already configured logical pins. Pin numbering and MCU setup remain
 * backend responsibilities.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 *
 * @copyright Copyright (c) 2026 Axel Napolitano
 *
 * @license PolyForm Noncommercial License 1.0.0
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

namespace eurorack::io {

/**
 * @brief Read-only logical digital input.
 */
class DigitalInput {
  public:
    /**
     * @brief Destroys a digital-input backend through its interface.
     */
    virtual ~DigitalInput() = default;

    /**
     * @brief Reads the current logical input level.
     *
     * @return True for a logical high level; false for a logical low level.
     *
     * Electrical polarity, pull resistors, and threshold conversion are backend
     * concerns. This method performs one immediate sample and does not debounce.
     */
    [[nodiscard]] virtual bool readHigh() const noexcept = 0;
};

/**
 * @brief Writable logical digital output.
 */
class DigitalOutput {
  public:
    /**
     * @brief Destroys a digital-output backend through its interface.
     */
    virtual ~DigitalOutput() = default;

    /**
     * @brief Writes a logical output level.
     *
     * @param high True to request a logical high level; false for logical low.
     *
     * The backend is responsible for translating the logical request into the
     * platform-specific register or API call.
     */
    virtual void writeHigh(bool high) noexcept = 0;

    /**
     * @brief Returns the most recently requested logical output level.
     *
     * @return True when the last requested level was high; otherwise false.
     *
     * The value represents the software request and does not prove the physical
     * voltage present at the pin.
     */
    [[nodiscard]] virtual bool lastWrittenHigh() const noexcept = 0;
};

/**
 * @brief Bidirectional pin supporting explicit direction changes.
 */
class BidirectionalDigitalPin : public DigitalInput, public DigitalOutput {
  public:
    /**
     * @brief Available operating directions for a bidirectional pin.
     */
    enum class Direction {
        Input, ///< Pin is configured as an input.
        Output ///< Pin is configured as an output.
    };

    /**
     * @brief Destroys a bidirectional pin backend through its interface.
     */
    ~BidirectionalDigitalPin() override = default;

    /**
     * @brief Changes the pin direction.
     *
     * @param direction Requested input or output mode.
     */
    virtual void setDirection(Direction direction) noexcept = 0;

    /**
     * @brief Returns the current configured pin direction.
     *
     * @return Current input or output mode.
     */
    [[nodiscard]] virtual Direction direction() const noexcept = 0;
};

} // namespace eurorack::io
