// Contains pin definitions for espcam
#pragma once

//available pin list

//2, 4, 12, 13, 14, 15

//led 33


// sx1278 pin defs
constexpr int sxSCK_pin = 14;
constexpr int sxMOSI_pin = 15;
constexpr int sxMISO_pin = 2;
constexpr int sxNSS_pin = 13;
constexpr int sxRESET_pin = -1;

constexpr int sxDIO0_pin = 12;
constexpr int sxDIO1_pin = -1;
constexpr int sxDIO2_pin = 4;

// GPIO16 on this board is connected to onboard PSRAM. Using this GPIO for other purposes will trigger the watchdog

// gps pin defs
constexpr int gpsRXPin = 3;

constexpr int  GPSBaud = 9600;
