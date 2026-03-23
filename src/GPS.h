#ifndef GPS_H
#define GPS_H

void UpdateGPSInfo(
  float &lat, float &lng, float &age_s,                 // location + age (s)
  uint16_t &year, uint8_t &month, uint8_t &day,         // date (UTC)
  uint8_t &hour, uint8_t &minute, uint8_t &second, uint8_t &centisecond, // time (UTC)
  float &alt_m,                                         // altitude (m)
  float &speed_kmh,                                     // speed (km/h)
  float &course_deg,                                    // course (deg)
  uint8_t &sats,                                        // satellite count
  float &hdop                                           // HDOP
);

extern TinyGPSPlus gps;

#endif