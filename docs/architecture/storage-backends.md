# Concrete Storage Backends

Step 13 adds two implementations of `PersistentStorage`.

`FileStorage` is intended for native tests, simulators, and desktop tools. It
loads a fixed-size byte image into memory and stages changes until `commit()`.
Commit writes the image to a temporary file and then replaces the target file.

`ArduinoEepromStorage` exposes a bounded region of Arduino EEPROM. Writes and
erase operations use `EEPROM.update()` to avoid rewriting unchanged cells.

On AVR and comparable cores, commit is immediate and therefore a no-op. On
ESP8266 and ESP32, `begin()` and `commit()` call the EEPROM emulation API.
