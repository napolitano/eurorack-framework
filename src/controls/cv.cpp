/**
 * @file src/controls/cv.cpp
 * @brief Implements voltage-domain CV input and output models.
 *
 * @details
 * Implements calibrated control-voltage conversion.
 *
 * The model separates raw converter codes from volts and normalized values, applies explicit
 calibration, and reports clipping. It does not perform ADC acquisition or DAC output itself.

 * @author Axel Napolitano
 * @date 2026
 * @par Contact
 * eurorack\@skjt.de
 * @par License
 * PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 * @see LICENSE and ADDITIONAL_PERMISSION.md in the repository root.
 */

#include <eurorack/controls/cv.hpp>

namespace eurorack::controls {

/*
 * Purpose: Constructs a CV input model.
 *
 * range: Accepted operational voltage range.
 */
CvInput::CvInput(const eurorack::core::VoltageRange range) noexcept : range_(range) {}

/*
 * Purpose: Processes one calibrated voltage sample.
 *
 * volts: Measured voltage after hardware calibration.
 */
void CvInput::update(const float volts) noexcept {
    snapshot_.rawVolts = volts;
    snapshot_.belowRange = volts < range_.minimumVolts;
    snapshot_.aboveRange = volts > range_.maximumVolts;
    snapshot_.volts = range_.clamp(volts);

    const float span = range_.spanVolts();
    snapshot_.normalized = span > 0.0F ? (snapshot_.volts - range_.minimumVolts) / span : 0.0F;
}

/*
 * Purpose: Returns the current immutable CV-input state.
 *
 * Returns: Constant reference to the latest voltage-domain state.
 */
const CvInputSnapshot& CvInput::snapshot() const noexcept {
    return snapshot_;
}

/*
 * Purpose: Constructs a CV output model.
 *
 * range: Permitted output-voltage range.
 */
CvOutput::CvOutput(const eurorack::core::VoltageRange range) noexcept : range_(range) {}

/*
 * Purpose: Requests an output voltage.
 *
 * volts: Requested voltage.
 */
void CvOutput::setVolts(const float volts) noexcept {
    snapshot_.requestedVolts = volts;
    snapshot_.clampedLow = volts < range_.minimumVolts;
    snapshot_.clampedHigh = volts > range_.maximumVolts;
    snapshot_.effectiveVolts = range_.clamp(volts);
}

/*
 * Purpose: Returns the current immutable CV-output state.
 *
 * Returns: Constant reference to requested and effective output voltage.
 */
const CvOutputSnapshot& CvOutput::snapshot() const noexcept {
    return snapshot_;
}

} // namespace eurorack::controls
