#include "RLNC.h"
#include "RLNC_Tables.h"
#include <cstring> // For memset

TinyRLNC::TinyRLNC(const uint8_t* data_ptr) : _data_ptr(data_ptr) {}

// PRNG: Xorshift32
uint32_t TinyRLNC::xorshift32(uint32_t& state) {
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;
    return x;
}

// GF Multiplication
// Uses the precomputed tables from TinyRLNC_Tables.h
inline uint8_t TinyRLNC::gf_mul(uint8_t a, uint8_t b) {
    if (a == 0 || b == 0) return 0;
    // gf_exp is size 512, so gf_log[a] + gf_log[b] (max 255+255=510) is safe
    return gf_exp[gf_log[a] + gf_log[b]];
}

void TinyRLNC::encode(uint32_t seed, uint8_t* out_buffer) {
    // 1. Initialize Buffer
    std::memset(out_buffer, 0, RLNC_BLOCK_SIZE);

    // 2. Initialize PRNG State
    // Handle 0 seed edge case for Xorshift
    uint32_t state = (seed == 0) ? 1 : seed;

    // 3. Encoding Loop (Optimized: Row-major)
    // Iterate through all 1600 source blocks
    for (int i = 0; i < RLNC_BLOCK_COUNT; ++i) {
        
        // a. Generate Random Coefficient
        // Update state and take lower 8 bits
        xorshift32(state); 
        uint8_t coeff = state & 0xFF;

        // Optimization: If coefficient is 0, skipping the block saves time.
        if (coeff == 0) continue;

        // b. Pointer to current source block
        const uint8_t* src = _data_ptr + (i * RLNC_BLOCK_SIZE);
        
        // c. Galois Field Summation
        // XOR the result of (BlockBytes * coeff) into the output buffer
        for (int j = 0; j < RLNC_BLOCK_SIZE; ++j) {
            uint8_t term = gf_mul(src[j], coeff);
            out_buffer[j] ^= term;
        }
    }
}