#include "LinearErasureCoder.h"
#include <LEC_tables.h>

// LCG Constants
static const uint32_t LCG_A = 1664525;
static const uint32_t LCG_C = 1013904223;

LinearErasureCoder::LinearErasureCoder(size_t packetSize) : _packetSize(packetSize) {}

uint8_t LinearErasureCoder::gf_mul(uint8_t a, uint8_t b) {
    if (a == 0 || b == 0) return 0;
    // gf_exp is size 512, so no modulo needed for the index sum (max 255+255=510)
    return gf_exp[(int)gf_log[a] + (int)gf_log[b]];
}

uint32_t LinearErasureCoder::hashSeeds(float a, float b, uint8_t c) {
    uint32_t ia, ib;
    // Raw bit copy to avoid strict aliasing violations or value normalization
    memcpy(&ia, &a, sizeof(float));
    memcpy(&ib, &b, sizeof(float));

    // Combine seeds
    uint32_t state = ia ^ ib ^ c;

    // Apply MurmurHash3 integer finalizer mix
    state ^= (state >> 16);
    state *= 0x85ebca6b;
    state ^= (state >> 13);
    state *= 0xc2b2ae35;
    state ^= (state >> 16);

    return state;
}

uint8_t LinearErasureCoder::getNextCoefficient(uint32_t* state) {
    uint8_t coeff = 0;
    do {
        *state = (*state) * LCG_A + LCG_C;
        coeff = (uint8_t)(*state >> 24); // Take high byte
    } while (coeff == 0); // Regenerate if 0 to maintain linear independence probability
    return coeff;
}

bool LinearErasureCoder::encodePacket(fs::FS &fs, const char* filename, float seedA, float seedB, uint8_t seedC, uint8_t* outputBuf) {
    File file = fs.open(filename, "r");
    if (!file) {
        return false;
    }

    size_t fileSize = file.size();
    if (fileSize == 0) {
        file.close();
        // Handle empty file: zero out buffer and return
        memset(outputBuf, 0, _packetSize);
        return true;
    }

    // Initialize accumulator buffer
    memset(outputBuf, 0, _packetSize);

    // Initialize PRNG
    uint32_t prngState = hashSeeds(seedA, seedB, seedC);

    // Calculate total blocks K
    size_t K = (fileSize + _packetSize - 1) / _packetSize;

    // Linear Combination Loop
    for (size_t i = 0; i < K; i++) {
        // 1. Generate Coefficient for this block
        uint8_t ci = getNextCoefficient(&prngState);

        // 2. Seek to start of block i
        // Note: Checking seek return might be necessary depending on FS implementation
        if (!file.seek(i * _packetSize)) {
            file.close();
            return false;
        }

        // 3. Accumulate Bytes
        // We read byte-by-byte to minimize stack usage
        // For higher performance (at cost of RAM), a small buffer (e.g., 64 bytes) could be used here.
        for (size_t j = 0; j < _packetSize; j++) {
            int byteRead = file.read();
            
            uint8_t b_val = 0;
            if (byteRead != -1) {
                b_val = (uint8_t)byteRead;
            } else {
                // End of file reached inside a block (padding)
                // b_val remains 0
            }

            // P_out[j] = P_out[j] ^ (Block[j] * ci)
            if (b_val != 0) { // Optimization: gf_mul(0, x) is always 0
                outputBuf[j] ^= gf_mul(b_val, ci);
            }
        }
    }

    file.close();
    return true;
}