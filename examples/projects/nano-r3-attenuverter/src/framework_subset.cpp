/**
 * @file framework_subset.cpp
 * @brief Selects the framework implementation units required by this AVR example.
 *
 * @details
 * PlatformIO normally compiles every source file in a library. The complete
 * framework intentionally contains desktop simulation, storage, display, and
 * other components that are not part of this small ATmega328P application and
 * that may depend on standard-library facilities unavailable on Arduino AVR.
 *
 * This internal example build shim includes only the three implementation units
 * used by `main.cpp`. Consuming projects on larger targets can link the complete
 * library or define their own explicit component selection. Application code
 * must continue to include only public headers.
 */

#include "../../../../src/controls/analog_input.cpp"
#include "../../../../src/controls/cv.cpp"
#include "../../../../src/drivers/dac/mcp4922.cpp"
