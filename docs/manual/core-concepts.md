# Core Concepts

## Models do not own hardware

Controls and signal models accept sampled values or non-owning references to
interfaces. They do not configure MCU pins and do not own referenced buses,
storage backends, clocks, or converters.

Referenced dependencies must outlive the objects that use them.

## Raw codes and engineering units are separate

ADC and DAC interfaces expose raw integer codes. Calibration converts between
codes and volts. Application logic should not assume that a raw code maps
perfectly to an ideal voltage.

## Transient events have a limited lifetime

Fields such as `justPressed`, `justReleased`, `changed`, and encoder `delta`
normally describe only the most recent update. Copy them into an application
event queue when events must survive more than one update cycle.

## Synchronization points are explicit

Buffered drivers update software state first. `flush()` or `commit()` is the
explicit point where state is transferred to hardware or persistent media.

## Native simulation is not embedded firmware

Native components may use dynamic memory, files, exceptions, and standard
library containers. Embedded components avoid those dependencies unless
explicitly documented.

## Successful bus traffic is not physical verification

A successful SPI or I2C call proves only that the software bus backend accepted
the transaction. It does not prove correct voltage, timing margin, optical
output, analog accuracy, or physical device presence.
