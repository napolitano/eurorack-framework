# Analog multiplexer driver

## Purpose

`eurorack-driver-analog-multiplexer` is the software driver for **74hc4051-74hc4067**. This page documents the framework-facing contract. Device electrical behavior and board-level integration are documented separately in [docs/hardware/multiplexer/74hc4051-74hc4067.md](../hardware/multiplexer/74hc4051-74hc4067.md).

## Public API

- Public header: `eurorack/drivers/mux/analog_multiplexer.hpp`
- Primary type: `AnalogMultiplexer<SelectLineCount>`
- Maturity: `alpha`

- `channelCount` is available at compile time.
- `selectChannel()` masks the requested channel to the supported address width and waits for settling.
- `readChannel()` selects and samples in one synchronous operation.
- `readRaw()` samples the currently selected channel through the common ADC.

The generated Doxygen reference remains authoritative for exact signatures, parameter types, enumerators, and return annotations.

## Lifecycle and configuration

Instantiate the template with the number of address lines, provide select outputs least-significant bit first, a common ADC input, a delay provider, and a settling interval.

Constructors do not transfer ownership of framework adapters. Configuration values are captured by value unless the public header explicitly accepts a pointer or reference. No implicit global initialization is performed.

## Ownership and memory

All dependencies are borrowed. The class has fixed-size storage and performs no dynamic allocation. Null select-line pointers are skipped and are only valid when the corresponding address line is fixed externally.

Copy and move behavior follows the declarations in the public header. Applications should normally create one long-lived driver instance per physical device or bus address.

## Error handling

The returned `AnalogSample` carries the underlying ADC status. Out-of-range channel values are masked rather than rejected.

Callers shall inspect every returned `eurorack::io::IoResult` or `AnalogSample::result`. The driver does not log, throw exceptions, or silently perform unbounded retries.

## ISR, reentrancy, and concurrency

This driver is not reentrant. Synchronous foreground use only. Channel selection and sampling are not atomic and the class is not reentrant. Blocking delay makes ISR use inappropriate.

Unless a future API explicitly states otherwise, this library makes no thread-safety guarantee. Shared buses must be arbitrated above the driver layer.

## Dependencies and resources

The library depends only on the granular framework interfaces declared by its `library.json`. Hardware resources are represented by injected adapters rather than selected globally. Refer to the [hardware page](../hardware/multiplexer/74hc4051-74hc4067.md) and the framework resource registry for concrete bus and GPIO requirements.

## Usage example

The executable example is located at `examples/components/analog-multiplexer`:

```cpp
#include "../driver_example_support.hpp"
#include <array>
#include <eurorack/drivers/mux/analog_multiplexer.hpp>
int main(){ example::DigitalOutput s0,s1,s2; example::AnalogInput adc(3072U); example::Delay delay; std::array<eurorack::io::DigitalOutput*,3U> select{&s0,&s1,&s2}; eurorack::drivers::mux::AnalogMultiplexer<3U> mux(select,adc,delay,5U); const auto sample=mux.readChannel(5U); return sample.result==eurorack::io::IoResult::Success && sample.code==3072U && mux.currentChannel()==5U ? 0:1; }
```

The repository example, not this excerpt, is the build-checked source of truth.

## Tests

Mapped native test suites:

- `tests/native/test_driver_74hc4051_4067`

A test mapping proves that the suite exercises the library; it does not by itself claim exhaustive line or branch coverage.

## Limitations

- No internal scheduler, locking, or background transfer engine is provided.
- Electrical limits, decoupling, level compatibility, and analog accuracy remain hardware-design responsibilities.
- Unsupported silicon features are listed on the hardware page.
- API compatibility is not yet guaranteed because the framework remains alpha.

## Related documentation

- [Hardware guide](../hardware/multiplexer/74hc4051-74hc4067.md)
- [Documentation standard](../standards/documentation-standard.md)
- Public header: `libraries/eurorack-driver-analog-multiplexer/include/eurorack/drivers/mux/analog_multiplexer.hpp`
- Example: `examples/components/analog-multiplexer`
