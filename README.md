# HAB Tracker - High Altitude Balloon Flight Computer

ESP32-CAM flight computer for HAB missions. Transmits APRS telemetry over 2m AFSK (AX.25) and LoRa, captures JPEGs at altitude, and downlinks them using GF(2^8) RLNC erasure coding. Radio parameters are selected automatically by geofence.

---

## Features

**Dual-radio APRS** - Alternates between 2m AFSK via RadioLib's AX.25 stack and LoRa APRS on a single SX1278. Frequency, SF, and coding rate are chosen by GPS position to match local band plans (Europe, Americas, China, Japan, Australia/NZ, etc.).

**RLNC image transport** - Each TX sends a GF(2^8) random linear combination of all 53-byte blocks of a stored JPEG rather than sequential chunks. Coefficients are seeded from the balloon's APRS coordinate strings for implicit metadata and reproducible vectors; payloads are base91-encoded for compact APRS message bodies. The ground station can reconstruct from any sufficient set of linearly independent packets.

**Reception-aware accounting** - When over a known iGate coverage area (continental Europe, UK, coastal USA, Japan, eastern China, Scandinavia), ACKs parsed from downlinked LoRa packets decrement each image's remaining-packet counter. Images are deleted from LittleFS once fully acknowledged. Counters persist across reboots via NVS Preferences.

**Quality zones** - Configurable polygons over visually interesting regions trigger a high-priority capture on zone entry and shorten the inter-image interval for the next orbit.

---

## Hardware

| Component | Details |
|---|---|
| ESP32-CAM (AI-Thinker) | Main processor, PSRAM, OV2640, LittleFS |
| SX1278 | SPI, handles both LoRa and 2m AFSK via RadioLib |
| GPS module | NMEA 9600 baud, TinyGPS+ |

## Dependencies

- [RadioLib](https://github.com/jgromes/RadioLib)
- [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus)
- Arduino ESP32 core (LittleFS, Preferences, esp_camera)

---

## Project Structure

| File | Purpose |
|---|---|
| `main.cpp` | Main loop: GPS, scheduling, image capture, packet dispatch |
| `radio.cpp` / `.h` | TX helpers, LoRa RX, packet accounting |
| `geofence.cpp` / `.h` | Worldwide APRS frequency geofencing |
| `quality.cpp` / `.h` | Interesting-zone detection |
| `LinearErasureCoder.cpp` / `.h` | GF(2^8) RLNC encoder |
| `LEC_tables.h` | Pre-computed GF log/exp tables |
| `base91.cpp` / `.h` | Base91 + APRS lat/lng formatting |
| `camutils.cpp` / `.h` | Camera init, JPEG capture, LittleFS storage |
| `GPS.cpp` / `.h` | TinyGPS+ wrapper |
| `pin_defs.h` | GPIO assignments |

---

## Configuration

Timing constants at the top of `main.cpp`:

```cpp
const unsigned telempacketinterval = 20;       // Telemetry packet every N packets
const unsigned long TX_INTERVAL     = 5000;    // ms between transmissions
const unsigned long IMG_interval    = 3600000; // ms between image captures
const unsigned updateprefs          = 600000;  // ms between NVS counter syncs
```

Pin assignments for the SX1278 and GPS in `pin_defs.h`.

---

## Image TX Flow

1. JPEG saved to LittleFS with GPS/timestamp metadata trailer; packet counter assigned.
2. Each TX cycle: RLNC encoder produces a 53-byte coded packet seeded by current APRS position strings + random identifier byte.
3. 54-byte payload base91-encoded, sent as APRS message body.
4. Counter decremented on ACK (if over iGate coverage). File deleted at zero.
5. Ground station reconstructs once it holds enough linearly independent packets.
