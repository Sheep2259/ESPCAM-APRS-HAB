#ifndef BASE91_H
#define BASE91_H

void encodeBase91(const uint8_t* data, size_t len, char* output);

void aprsFormatLat(float lat, char* buf, size_t bufSize);
void aprsFormatLng(float lat, char* buf, size_t bufSize);

uint8_t packBools(bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7);


#endif
