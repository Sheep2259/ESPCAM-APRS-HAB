#ifndef LINEAR_ERASURE_CODER_H
#define LINEAR_ERASURE_CODER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

// External Galois Field lookup tables (must be defined in your .cpp or a separate data file)
extern const uint8_t gf_log[256];
extern const uint8_t gf_exp[512];

class LinearErasureCoder {
public:
    /**
     * Configures the coder.
     * @param packetSize The fixed size (in bytes) of the output packet payload.
     */
    LinearErasureCoder(size_t packetSize);

    /**
     * Generates a single encoded packet using RLNC.
     * * @param fs Reference to the active LittleFS file system.
     * @param filename Path to the source file.
     * @param seedA Input float 1 (Part of prng seed).
     * @param seedB Input float 2 (Part of prng seed).
     * @param seedC Input uint8_t (Part of prng seed).
     * @param outputBuf Pointer to caller-allocated buffer of size `packetSize`.
     * @return true if successful, false on file error.
     */
    bool encodePacket(fs::FS &fs, const char* filename, float seedA, float seedB, uint8_t seedC, uint8_t* outputBuf);

private:
    size_t _packetSize;

    // Internal helpers
    uint8_t gf_mul(uint8_t a, uint8_t b);
    uint32_t hashSeeds(float a, float b, uint8_t c);
    uint8_t getNextCoefficient(uint32_t* state);
};

#endif // LINEAR_ERASURE_CODER_H