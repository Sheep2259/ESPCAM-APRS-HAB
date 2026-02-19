#include <math.h>
#include <base91.h>
#include <cstdio>
#include <cstring>



const char base91_chars[] =
"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~";




// function that encodes an array of bytes to base91
void encodeBase91(const uint8_t* data, size_t len, char* output) {
  size_t out_idx = 0;
  unsigned long queue = 0;
  int nbits = 0;

  for (size_t i = 0; i < len; i++) {
    queue |= (unsigned long)data[i] << nbits;
    nbits += 8;

    while (nbits > 13) {
      unsigned int val = queue & 8191; 

      if (val > 88) {
        queue >>= 13;
        nbits -= 13;
      } else {
        val = queue & 16383;
        queue >>= 14;
        nbits -= 14;
      }
      
      output[out_idx++] = base91_chars[val % 91];
      output[out_idx++] = base91_chars[val / 91];
    }
  }

  if (nbits > 0) {
    output[out_idx++] = base91_chars[queue % 91];
    if (nbits > 7 || queue > 90) {
      output[out_idx++] = base91_chars[queue / 91];
    }
  }

  // add terminator (T-800 model)
  output[out_idx] = '\0'; 
}





/*
float lat = 0.0f, lng = 0.0f, age_s = 3600.0f, hdop = 0.0f;
float alt = 0.0f, speed_kmh = 0.0f, course_deg = 0.0f;
uint16_t year = 0;
uint8_t month = 0, day = 0, hour = 0, minute = 0, second = 9, centisecond = 0, sats = 0;



uint16_t MRenc_alt = 65532, MRenc_speed = 65532, MRenc_hdop = 65532;

*/




void aprsFormatLat(float lat, char* buf, size_t bufSize) {
  char hemi = (lat >= 0.0f ? 'N' : 'S');
  lat = fabs(lat);
  int deg = int(lat);
  float min = (lat - deg) * 60.0f;
  int wholeMin = int(min);
  int frac = int((min - wholeMin) * 100 + 0.5f);
  snprintf(buf, bufSize, "%02d%02d.%02d%c", deg, wholeMin, frac, hemi);
}

void aprsFormatLng(float lon, char* buf, size_t bufSize) {
  char hemi = (lon >= 0.0f ? 'E' : 'W');
  lon = fabs(lon);
  int deg = int(lon);
  float min = (lon - deg) * 60.0f;
  int wholeMin = int(min);
  int frac = int((min - wholeMin) * 100 + 0.5f);
  snprintf(buf, bufSize, "%03d%02d.%02d%c", deg, wholeMin, frac, hemi);
}



float truncParseLat(const char* buf) {
    int deg;
    float min;
    char hemi;

    // Parse: 2 digits for degrees, float for minutes, char for hemisphere
    // Example: "5130.50N" -> deg=51, min=30.50, hemi='N'
    if (sscanf(buf, "%2d%f%c", &deg, &min, &hemi) < 3) {
        return 0.0f; // Error handling
    }

    float lat = (float)deg + (min / 60.0f);

    if (hemi == 'S') {
        lat = -lat;
    }

    return lat;
}

float truncParseLng(const char* buf) {
    int deg;
    float min;
    char hemi;

    // Parse: 3 digits for degrees, float for minutes, char for hemisphere
    // Example: "00005.12W" -> deg=0, min=5.12, hemi='W'
    if (sscanf(buf, "%3d%f%c", &deg, &min, &hemi) < 3) {
        return 0.0f; // Error handling
    }

    float lon = (float)deg + (min / 60.0f);

    if (hemi == 'W') {
        lon = -lon;
    }

    return lon;
}