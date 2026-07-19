# Persistence Architecture

The persistence layer stores explicit byte payloads. It never writes arbitrary
C++ object layouts directly.

## Layers

- `PersistentStorage` - byte-addressed backend contract
- `MemoryStorage` - native-test and simulator backend
- `ByteWriter` / `ByteReader` - explicit little-endian serialization
- `crc32()` - payload and metadata integrity
- `RecordStore` - dual-slot atomic records with sequence numbers

## Atomic update model

Each record region contains two equal slots. A new payload is written to the
inactive slot. The payload is written first, then the validated header, then the
backend is committed.

On load, both slots are inspected. The newest valid sequence is selected. If the
newest slot is corrupt, the previous valid slot remains available.

## Versioning

The record header stores a schema version. `RecordStore` reports a version
mismatch rather than interpreting incompatible bytes. A consuming project owns
its serializer and migration functions.

## Wear leveling

The dual-slot store reduces the risk of interrupted writes but is not full wear
leveling. A later wear-leveling layer may distribute records across more than
two slots without changing serializers or backend interfaces.
