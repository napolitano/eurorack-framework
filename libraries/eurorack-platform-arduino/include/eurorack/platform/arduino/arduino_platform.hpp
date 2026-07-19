/**
 * @file include/eurorack/platform/arduino/arduino_platform.hpp
 * @brief Includes all Arduino platform adapters.
 *
 * @details
 * Convenience umbrella header for GPIO, analog, time, SPI, and I2C adapters.
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
 * @ingroup platform_arduino
 */

#pragma once

#ifdef ARDUINO

#include <eurorack/platform/arduino/arduino_analog.hpp>
#include <eurorack/platform/arduino/arduino_eeprom_storage.hpp>
#include <eurorack/platform/arduino/arduino_gpio.hpp>
#include <eurorack/platform/arduino/arduino_i2c.hpp>
#include <eurorack/platform/arduino/arduino_spi.hpp>
#include <eurorack/platform/arduino/arduino_time.hpp>

#endif // ARDUINO
