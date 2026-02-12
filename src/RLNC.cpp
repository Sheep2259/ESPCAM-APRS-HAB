#include "RLNC.h"
#include "RLNC_Tables.h"
#include <cstring>

TinyRLNC::TinyRLNC(const uint8_t* data_ptr, size_t total_bytes) 
    : _data_ptr(data_ptr), _total_bytes(total_bytes) 
{
    // 1. Calculate how many FULL 50-byte blocks we have
    _full_blocks = total_bytes / RLNC_BLOCK_SIZE;

    // 2. Calculate if there is a partial block at the end
    _partial_bytes = total_bytes % RLNC_BLOCK_SIZE;
}

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
    std::memset(out_buffer, 0, RLNC_BLOCK_SIZE);
    
    // Handle 0 seed edge case
    uint32_t state = (seed == 0) ? 1 : seed;

    // --- PHASE 1: Process all FULL blocks (Fast Path) ---
    for (uint32_t i = 0; i < _full_blocks; ++i) {
        
        // Generate Coefficient
        uint32_t x = state;
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        state = x;
        uint8_t coeff = x & 0xFF;

        if (coeff == 0) continue;

        // Pointer to this block
        const uint8_t* src = _data_ptr + (i * RLNC_BLOCK_SIZE);

        // XOR full 50 bytes
        for (int j = 0; j < RLNC_BLOCK_SIZE; ++j) {
            out_buffer[j] ^= gf_exp[gf_log[src[j]] + gf_log[coeff]];
        }
    }

    // --- PHASE 2: Process the final PARTIAL block (Safe Path) ---
    if (_partial_bytes > 0) {
        // Generate Coefficient for the last block
        uint32_t x = state;
        x ^= x << 13; x ^= x >> 17; x ^= x << 5;
        state = x;
        uint8_t coeff = x & 0xFF;

        if (coeff != 0) {
            const uint8_t* src = _data_ptr + (_full_blocks * RLNC_BLOCK_SIZE);

            // ONLY read the valid bytes
            for (int j = 0; j < _partial_bytes; ++j) {
                out_buffer[j] ^= gf_exp[gf_log[src[j]] + gf_log[coeff]];
            }
            // The remaining bytes (j >= _partial_bytes) are effectively 
            // treated as 0. Since 0 * coeff = 0, and X ^ 0 = X, 
            // we simply do nothing.
        }
    }
}