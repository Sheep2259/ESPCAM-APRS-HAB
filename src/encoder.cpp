#include <BigNumber.h>
#include <tuple>
#include <vector>


// THIS IS ALL UNTESTED

#include <BigNumber.h>

const char base91_chars[] =
    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~";

String toBase91(BigNumber n) {
  if (n == 0) return String(base91_chars[0]);
  BigNumber base = 91;
  String result = "";

  while (n > 0) {
    BigNumber quotient = n / base;
    BigNumber remainder = n - quotient * base;  // manual modulo

    // Convert remainder -> int safely via string
    String rStr = remainder.toString();
    int rem = rStr.toInt();  // guaranteed < 91

    result = String(base91_chars[rem]) + result;
    n = quotient;
  }

  return result;
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





BigNumber encodeMixedRadix(const std::vector<std::tuple<uint16_t, uint16_t>>& digits_and_bases)
{
    BigNumber encoded = 0;
    BigNumber multiplier = 1;

    for (const auto& [digit, base] : digits_and_bases)
    {
        encoded += static_cast<BigNumber>(digit) * multiplier;
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
                      uint16_t batvoltage, uint16_t PVvoltage,
                      uint16_t *enc_alt, uint16_t *enc_speed,
                      uint16_t *enc_hdop, uint16_t *enc_bat, uint16_t *enc_pv) {

  *enc_alt  = alt / 20;
  *enc_speed = speed_kmh / 2;
  *enc_hdop  = hdop * 10;
  *enc_bat   = batvoltage / 10;
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
