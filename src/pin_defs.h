// Contains pin definitions for espcam
#pragma once


// sx1278 pin defs
constexpr int sxSCK_pin = 14;
constexpr int sxMOSI_pin = 15;
constexpr int sxMISO_pin = 2;
constexpr int sxNSS_pin = 13;

constexpr int sxDIO0_pin = 12;
constexpr int sxDIO1_pin = -1;
constexpr int sxDIO2_pin = 4;


// gps pin defs
constexpr int gpsTXPin = -1;
constexpr int gpsRXPin = 16;

constexpr int  GPSBaud = 9600;

// other pin defs
constexpr int vsensesolar_pin = 0; // uses 0.5x voltage divider
