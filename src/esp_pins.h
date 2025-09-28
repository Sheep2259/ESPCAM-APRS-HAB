// Contains pin definitions for esp32
#pragma once


// SX1278 has the following connections:
// NSS pin:   16
// DIO0 pin:  2
// RESET pin: 17
// DIO1 pin:  5
// SX1278 radio = new Module(16, 2, 17, 5);
// For 2m aprs, uses DIO2 connected to 21


// sx1278 pin defs, have to connect rest of SPI pins
constexpr int sxNSS_pin = 16;
constexpr int sxDIO0_pin = 2;
constexpr int sxRESET_pin = 17;
constexpr int sxDIO1_pin = 15;
constexpr int sxDIO2_pin = 21;


// gps pin defs
constexpr int gpsTX_pin = 32;
constexpr int gpsRX_pin = 33;


// other pin defs
constexpr int vsense_pin = 27;
