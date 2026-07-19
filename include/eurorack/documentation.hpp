/**
 * @file include/eurorack/documentation.hpp
 * @brief Defines the public Doxygen module hierarchy.
 *
 * @details
 * This header contains documentation-only groups. It has no runtime behavior,
 * owns no resources, and introduces no executable code into consuming firmware.
 *
 * @author Axel Napolitano
 * @date 2026
 * @contact eurorack@skjt.de
 * @license PolyForm Noncommercial License 1.0.0
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */

#pragma once

/**
 * @defgroup controls Control state machines and user-facing hardware abstractions
 *
 * @details
 * Control objects translate raw hardware observations into stable, application-facing state.
 * They do not own referenced hardware interfaces; dependencies must outlive the control object.
 * Unless stated otherwise, calls are synchronous, allocate no memory, and are intended for a cooperative firmware loop.
 * Objects are not internally synchronized and require external protection when shared between interrupt and foreground contexts.
 */

/**
 * @defgroup core Framework configuration and foundational types
 *
 * @details
 * Core declarations define shared policy, validation, and configuration used by multiple framework layers.
 * Values use explicit units and ranges wherever practical.
 * Changing configuration may alter electrical thresholds, timing, memory requirements, and persistent behavior.
 */

/**
 * @defgroup display Display-independent graphics, text, widgets, and controller interfaces
 *
 * @details
 * Rendering operates on caller-owned canvas memory and uses integer pixel coordinates.
 * Drawing respects the active clip rectangle and performs no display transfer unless a concrete controller driver is called.
 * Canvas memory must not be modified concurrently during rendering or transfer.
 */

/**
 * @defgroup drivers Peripheral integrated-circuit drivers
 *
 * @details
 * Drivers consume abstract bus and pin interfaces and remain independent of a particular MCU SDK.
 * Referenced dependencies are non-owning and must outlive the driver.
 * A successful bus transaction does not validate the external circuit, reference voltage, current limiting, protection, or calibration.
 */

/**
 * @defgroup io Hardware-independent electrical, timing, and bus contracts
 *
 * @details
 * Interfaces define the contracts consumed by controls, drivers, platform adapters, and simulation.
 * Implementations must preserve documented units, ordering, ranges, and error semantics.
 * Frequently called implementations should avoid hidden allocation and unbounded blocking.
 */

/**
 * @defgroup platform_arduino Arduino Core platform adapters
 *
 * @details
 * Adapters bridge framework contracts to Arduino Core APIs and are available only when ARDUINO is defined.
 * They do not add electrical protection, atomicity, real-time guarantees, or thread safety beyond the selected core.
 * Raw microcontroller pins must not be connected directly to Eurorack jacks without suitable hardware.
 */

/**
 * @defgroup simulation Deterministic native simulation and test doubles
 *
 * @details
 * Simulation types prioritize repeatability and observability over embedded memory constraints.
 * They may allocate standard-library containers and are intended for native tests and desktop tools.
 * They are excluded from Arduino builds and are not production real-time components.
 */

/**
 * @defgroup storage Persistent storage, encoding, integrity, and records
 *
 * @details
 * Storage APIs operate on explicit byte-addressed regions and expose commit semantics.
 * Callers must check every IoResult and must not serialize compiler-dependent object layouts.
 * Unless a backend states otherwise, write updates staged state and commit establishes durability.
 */

/**
 * @defgroup configuration User-editable framework configuration
 *
 * @details
 * Configuration values are compile-time operating assumptions expressed in explicit units.
 * They are not electrical absolute maximum ratings and do not replace protection, measurement, or calibration.
 * Changing them requires rebuilding consuming firmware and may require updated tests or stored-data migration.
 */
