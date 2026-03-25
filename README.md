HAB Tracker — High Altitude Balloon Flight Computer
An ESP32-CAM based flight computer for high altitude balloon (HAB) missions. It transmits live APRS telemetry and position packets over both 2m FM (AFSK AX.25) and LoRa, captures JPEG images at altitude, and sends them back to the ground using a custom erasure-coded radio protocol — all while automatically adapting its radio parameters to local regulations worldwide.
Features
Dual-radio APRS transmissions via a single SX1278 module. The board alternates between 2m AFSK (using RadioLib's AX.25/APRS stack) and LoRa APRS, with frequency, spreading factor, and coding rate all selected automatically based on the balloon's GPS coordinates.
Geofencing engine covers most of the world's APRS frequency plans — Europe, the Americas, China, Japan, Australia, New Zealand, and more. A point-in-polygon algorithm (adapted from Tomy2's geofence library) picks the correct 2m and LoRa APRS frequencies in real time as the balloon drifts across regions.
Image capture and transmission. The OV2640 camera takes JPEG images periodically and whenever the balloon enters a predefined "interesting" geographic zone (coastlines, mountain ranges, famous landmarks, etc). Up to 16 images can be queued in LittleFS flash storage, each tracked with a remaining-packet counter that persists across reboots via NVS Preferences.
Random Linear Network Coding (RLNC) image transport. Rather than sending raw image chunks sequentially, each transmission sends a GF(2⁸) random linear combination of all 53-byte blocks of a stored JPEG. The receiver can reconstruct the original image from any sufficiently large set of received coded packets, regardless of order or gaps — making it well suited to the lossy, intermittent links typical of balloon-to-ground radio. Packet coefficients are seeded from the balloon's APRS-formatted coordinates, providing implicit metadata and reproducible coefficient vectors. Encoded packets are serialised to base91 for compact APRS message payloads.
Reception-aware packet accounting. A separate geofence layer maps known APRS iGate coverage areas (continental Europe, the UK, coastal USA, Japan, eastern China, Scandinavia). When the balloon is over a covered area, received-packet acknowledgements are parsed from downlinked LoRa packets and the remaining-packet counter for each image is decremented accordingly. Images are deleted from flash once fully acknowledged.
Quality zones trigger opportunistic imaging. A configurable list of polygons covering visually interesting regions automatically triggers a high-priority image capture on zone entry, with a shortened inter-image interval for the subsequent orbit.
Hardware

ESP32-CAM (AI-Thinker) — main processor, PSRAM, OV2640 camera, LittleFS on-chip flash
SX1278 LoRa transceiver — connected via SPI, used for both LoRa and 2m AFSK via RadioLib's FSK/AX.25 support
GPS module — NMEA at 9600 baud, parsed with TinyGPS+

