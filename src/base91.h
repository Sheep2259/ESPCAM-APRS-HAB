#ifndef BASE91_H
#define BASE91_H

#include <BigNumber.h>
#include <Arduino.h>




// Define a simple structure for our mixed radix pairs
struct RadixItem {
    uint16_t value;
    uint16_t base;
};

// Define how many items we have (6 items in your list)
#define PAYLOAD_ITEMS 6


BigNumber encodeMixedRadix(RadixItem* items, size_t count);
void toBase91(BigNumber n, char* outBuf, size_t bufSize);
void encodeBase91(const uint8_t* data, size_t len, char* output);


void MRencode_convert(float hdop, float alt, float speed_kmh, float course_deg,
                        uint16_t PVvoltage, uint16_t *enc_alt, uint16_t *enc_speed,
                        uint16_t *enc_hdop, uint16_t *enc_bat, uint16_t *enc_pv);


void aprsFormatLat(float lat, char* buf, size_t bufSize);
void aprsFormatLng(float lat, char* buf, size_t bufSize);

float truncParseLat(const char* buf);
float truncParseLng(const char* buf);


#endif // ENCODE_MIXED_RADIX_H
