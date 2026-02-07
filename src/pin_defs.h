// Contains pin definitions for espcam
#pragma once


// SX1278 has the following connections:
// NSS pin:   16
// DIO0 pin:  2
// RESET pin: 17
// DIO1 pin:  5
// SX1278 radio = new Module(16, 2, 17, 5);
// For 2m aprs, uses DIO2 connected to 21


// sx1278 pin defs
constexpr int sxSCK_pin = 2;
constexpr int sxMOSI_pin = 3;
constexpr int sxMISO_pin = 0;
constexpr int sxCS_pin = 1;

constexpr int sxNSS_pin = 13;
constexpr int sxDIO0_pin = 14;
constexpr int sxRESET_pin = 15;
constexpr int sxDIO1_pin = 26;
constexpr int sxDIO2_pin = 27;


// gps pin defs
constexpr int gpsTXPin = 4;
constexpr int gpsRXPin = 5;

constexpr int  GPSBaud = 9600;

// other pin defs
constexpr int vsensesolar_pin = 4; // uses 0.5x voltage divider
