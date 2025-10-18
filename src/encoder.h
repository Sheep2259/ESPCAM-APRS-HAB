// encode_mixed_radix.h
#ifndef ENCODE_MIXED_RADIX_H
#define ENCODE_MIXED_RADIX_H

#include <BigNumber.h>
#include <tuple>
#include <vector>

// Encodes a sequence of (digit, base) pairs into a single BigNumber
// using mixed radix representation.
//
// Parameters:
//   digits_and_bases - A vector of tuples, each containing a digit and its corresponding base.
//
// Returns:
//   A BigNumber representing the encoded mixed-radix value.

BigNumber encodeMixedRadix(const std::vector<std::tuple<uint16_t, uint16_t>>& digits_and_bases);

void MRencode_convert(float hdop, float alt, float speed_kmh, float course_deg,
                      uint16_t batvoltage, uint16_t PVvoltage,
                      uint16_t *enc_alt, uint16_t *enc_speed,
                      uint16_t *enc_hdop, uint16_t *enc_bat, uint16_t *enc_pv);

String aprsFormatLat(float lat, char* buf, size_t bufSize);
String aprsFormatLng(float lat, char* buf, size_t bufSize);

#endif // ENCODE_MIXED_RADIX_H
