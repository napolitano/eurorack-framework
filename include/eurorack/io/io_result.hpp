/**
 * @file include/eurorack/io/io_result.hpp
 * @brief Defines common status values returned by hardware interfaces.
 *
 * @details
 * The status type avoids exceptions and platform-specific error codes in embedded drivers.
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

#include <cstdint>

namespace eurorack::io {

/**
 * @brief Result of a hardware-interface operation.
 */
enum class IoResult : std::uint8_t {
    Success,         ///< The complete operation succeeded.
    InvalidArgument, ///< A supplied pointer, size, address, or setting was invalid.
    NotSupported,    ///< The backend does not support the requested operation.
    Busy,            ///< The resource is currently busy and may be retried.
    Timeout,         ///< The operation did not finish within its allowed time.
    NoDevice,        ///< No device acknowledged or responded.
    BusError,        ///< A bus-level protocol or electrical error occurred.
    DataError,       ///< Received or stored data failed validation.
    UnknownError     ///< The backend cannot classify the failure more precisely.
};

/**
 * @brief Reports whether an interface operation succeeded.
 *
 * @param result Operation result to inspect.
 * @return True only when result equals IoResult::Success; otherwise false.
 */
[[nodiscard]] constexpr bool succeeded(const IoResult result) noexcept {
    return result == IoResult::Success;
}

} // namespace eurorack::io
