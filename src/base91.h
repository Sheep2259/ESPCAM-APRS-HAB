#ifndef BASE91_H
#define BASE91_H

void encodeBase91(const uint8_t* data, size_t len, char* output);

void aprsFormatLat(float lat, char* buf, size_t bufSize);
void aprsFormatLng(float lat, char* buf, size_t bufSize);

float truncParseLat(const char* buf);
float truncParseLng(const char* buf);


#endif // ENCODE_MIXED_RADIX_H
