# HAB Tracker — High Altitude Balloon Flight Computer

An ESP32-CAM based flight computer for high altitude balloon (HAB) missions. It transmits live APRS telemetry and position packets over both 2m FM (AFSK AX.25) and LoRa, captures JPEG images at altitude, and sends them back to the ground using a custom erasure-coded radio protocol — all while automatically adapting its radio parameters to local regulations worldwide.

---

## Features

### Dual-Radio APRS Transmissions
Alternates between 2m AFSK (using RadioLib's AX.25/APRS stack) and LoRa APRS via a single SX1278 module. Frequency, spreading factor, and coding rate are all selected automatically based on the balloon's GPS coordinates.

### Worldwide Geofencing
A point-in-polygon engine covers most of the world's APRS frequency plans — Europe, the Americas, China, Japan, Australia, New Zealand, and more. The correct 2m and LoRa APRS frequencies are picked in real time as the balloon drifts across regions.

### Image Capture and Transmission
The OV2640 camera takes JPEG images periodically and whenever the balloon enters a predefined "interesting" geographic zone (coastlines, mountain ranges, famous landmarks, etc). Up to 16 images can be queued in LittleFS flash storage, each tracked with a remaining-packet counter that persists across reboots via NVS Preferences.

### RLNC Erasure-Coded Image Transport
Rather than sending raw image chunks sequentially, each transmission sends a GF(2⁸) random linear combination of all 53-byte blocks of a stored JPEG. The receiver can reconstruct the original image from any sufficiently large set of received coded packets, regardless of order or gaps — making it well suited to the lossy, intermittent links typical of balloon-to-ground radio.

Packet coefficients are seeded from the balloon's APRS-formatted coordinates, providing implicit metadata and reproducible coefficient vectors. Encoded packets are serialised to base91 for compact APRS message payloads.

### Reception-Aware Packet Accounting
A geofence layer maps known APRS iGate coverage areas (continental Europe, the UK, coastal USA, Japan, eastern China, Scandinavia). When over a covered area, received-packet acknowledgements are parsed from downlinked LoRa packets and the remaining-packet counter for each image is decremented. Images are deleted from flash once fully acknowledged.

### Quality Zones
A configurable list of polygons covering visually interesting regions triggers a high-priority image capture on zone entry, with a shortened inter-image interval for the subsequent orbit.

---

## Hardware

| Component | Details |
|---|---|
| ESP32-CAM (AI-Thinker) | Main processor, PSRAM, OV2640 camera, LittleFS on-chip flash |
| SX1278 LoRa transceiver | Connected via SPI, used for both LoRa and 2m AFSK via RadioLib |
| GPS module | NMEA at 9600 baud, parsed with TinyGPS+ |

---

## Dependencies

- [RadioLib](https://github.com/jgromes/RadioLib)
- [TinyGPS++](https://github.com/mikalhart/TinyGPSPlus)
- Arduino ESP32 core (LittleFS, Preferences, esp_camera)

---

## Project Structure

| File | Purpose |
|---|---|
| `main.cpp` | Main loop — GPS, scheduling, image capture, packet dispatch |
| `radio.cpp` / `radio.h` | Transmit helpers, LoRa RX, packet accounting |
| `geofence.cpp` / `geofence.h` | Worldwide APRS frequency geofencing |
| `quality.cpp` / `quality.h` | Interesting-location zone detection |
| `LinearErasureCoder.cpp` / `.h` | GF(2⁸) RLNC encoder |
| `LEC_tables.h` | Pre-computed GF log/exp tables |
| `base91.cpp` / `base91.h` | Base91 encoding + APRS lat/lng formatting |
| `camutils.cpp` / `camutils.h` | Camera init, JPEG capture, LittleFS storage |
| `GPS.cpp` / `GPS.h` | TinyGPS+ wrapper |
| `pin_defs.h` | GPIO assignments |

---

## Configuration

Key timing and behavioural constants are defined at the top of `main.cpp`:

```cpp
const unsigned telempacketinterval = 20;       // Telemetry packet every N packets
const unsigned long TX_INTERVAL     = 5000;    // Transmit every 5 seconds (ms)
const unsigned long IMG_interval    = 3600000; // Capture image every 1 hour (ms)
const unsigned updateprefs          = 600000;  // Sync counters to flash every 10 min (ms)
```

Pin assignments for the SX1278 and GPS module are in `pin_defs.h`.

---

## How Image Transmission Works

1. The camera captures a JPEG and saves it to LittleFS with a metadata trailer containing GPS coordinates and timestamp.
2. A packet counter is assigned to the image (default: number of encoded packets to send before considering it complete).
3. On each transmit cycle, the RLNC encoder reads the stored JPEG and produces a 53-byte coded packet seeded by the current APRS position strings and a random identifier byte.
4. The 54-byte payload (53 data + 1 identifier) is base91-encoded and sent as an APRS message body.
5. If the balloon is over a known iGate coverage area, the packet counter is decremented on transmission. When the counter reaches zero, the file is deleted from flash.
6. The ground station can reconstruct the image once it has received enough linearly independent coded packets.
