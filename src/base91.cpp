#include <BigNumber.h>
#include <base91.h>
#include <cstdio>
#include <cstring>



const char base91_chars[] =
"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~";


// function that encodes an integer to base91
void toBase91(BigNumber n, char* outBuf, size_t bufSize) {
    if (n == 0) {
        if (bufSize > 1) {
            outBuf[0] = base91_chars[0];
            outBuf[1] = '\0';
        }
        return;
    }

    BigNumber base = 91;
    char tempBuf[150]; // Temporary buffer for reversing
    int index = 0;

    // Generate digits (comes out in reverse order)
    while (n > 0 && index < (int)sizeof(tempBuf) - 1) {
        BigNumber quotient = n / base;
        BigNumber remainder = n - (quotient * base);

        // 1. Get the raw string pointer from BigNumber
        char* rawStr = remainder.toString(); 

        // 2. Convert to integer (remainder is always < 91, so atoi is safe)
        int rem = atoi(rawStr); 

        // 3. CRITICAL: Free the memory allocated by toString()
        free(rawStr); 

        tempBuf[index++] = base91_chars[rem];
        n = quotient;
    }

    // Reverse into the output buffer
    int outIdx = 0;
    for (int i = index - 1; i >= 0; i--) {
        if (outIdx < (int)bufSize - 1) {
            outBuf[outIdx++] = tempBuf[i];
        }
    }
    outBuf[outIdx] = '\0'; // Null terminate
}

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
BigNumber fromBase91(const String &s) {
  BigNumber base = 91;
  BigNumber result = 0;

  for (int i = 0; i < s.length(); i++) {
    char c = s[i];
    const char *pos = strchr(base91_chars, c);
    if (!pos) continue;  // ignore invalid chars
    BigNumber val((long)(pos - base91_chars));  // wrap in BigNumber
    result = result * base + val;
  }

  return result;
}
*/



BigNumber encodeMixedRadix(RadixItem* items, size_t count){
    BigNumber encoded = 0;
    BigNumber multiplier = 1;

    for (size_t i = 0; i < count; i++)
    {
        // Access fields directly from the struct
        uint16_t digit = items[i].value;
        uint16_t base  = items[i].base;

        // Safety modulo
        uint16_t safe_digit = digit % base;

        encoded += static_cast<BigNumber>(safe_digit) * multiplier;
        multiplier *= static_cast<BigNumber>(base);
    }

    return encoded;
}

/*
float lat = 0.0f, lng = 0.0f, age_s = 3600.0f, hdop = 0.0f;
float alt = 0.0f, speed_kmh = 0.0f, course_deg = 0.0f;
uint16_t year = 0;
uint8_t month = 0, day = 0, hour = 0, minute = 0, second = 9, centisecond = 0, sats = 0;



uint16_t MRenc_alt = 65532, MRenc_speed = 65532, MRenc_hdop = 65532;

*/
void MRencode_convert(float hdop, float alt, float speed_kmh, float course_deg,
                      uint16_t PVvoltage, uint16_t *enc_alt, uint16_t *enc_speed,
                      uint16_t *enc_hdop, uint16_t *enc_bat, uint16_t *enc_pv) {

  *enc_alt  = alt / 20;
  *enc_speed = speed_kmh / 2;
  *enc_hdop  = hdop * 10;
  *enc_pv    = PVvoltage / 10;

}




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