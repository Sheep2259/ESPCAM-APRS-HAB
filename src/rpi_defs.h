// Contains pin definitions for rpi 2040
#pragma once


// SX1278 has the following connections:
// NSS pin:   16
// DIO0 pin:  2
// RESET pin: 17
// DIO1 pin:  5
// SX1278 radio = new Module(16, 2, 17, 5);
// For 2m aprs, uses DIO2 connected to 21


// sx1278 pin defs
constexpr int sxSCK_pin = 2;  // 2, 6, 18, 22  } alternative pins
constexpr int sxMOSI_pin = 3;     // 3, 7, 19, 23  } 
constexpr int sxMISO_pin = 0;     // 0, 4, 16, 20  }
constexpr int sxCS_pin = 1;       // 1, 5, 17, 21  }

constexpr int sxNSS_pin = 13;
constexpr int sxDIO0_pin = 14;
constexpr int sxRESET_pin = 15;
constexpr int sxDIO1_pin = 26;
constexpr int sxDIO2_pin = 27;


// gps pin defs
constexpr int gpsTX_pin = 4;
constexpr int gpsRX_pin = 5;


// other pin defs
constexpr int vsensesolar_pin = 28; // uses 0.5x voltage divider
constexpr int vsensebat_pin = 29; // uses 0.5x voltage divider
