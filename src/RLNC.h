#ifndef TINY_RLNC_H
#define TINY_RLNC_H

#include <cstdint>

// Configuration
#define RLNC_BLOCK_SIZE 50    // Bytes per source block
#define RLNC_BLOCK_COUNT 1600 // Number of source blocks

class TinyRLNC {
public:
    /**
     * @brief Initialize the encoder.
     * @param data_ptr Pointer to the flat buffer containing all source data.
     * MUST be size: RLNC_BLOCK_COUNT * RLNC_BLOCK_SIZE (80KB).
     * The class does not copy this data; it reads it directly.
     */
    TinyRLNC(const uint8_t* data_ptr);

    /**
     * @brief Encodes a single packet.
     * @param seed A unique 32-bit integer (e.g., packet counter).
     * @param out_buffer Pointer to a buffer of size RLNC_BLOCK_SIZE (50 bytes).
     * The encoded payload will be written here.
     */
    void encode(uint32_t seed, uint8_t* out_buffer);

private:
    const uint8_t* _data_ptr;

    // Internal PRNG helper
    static uint32_t xorshift32(uint32_t& state);
    
    // Internal GF(2^8) helper using lookups
    static inline uint8_t gf_mul(uint8_t a, uint8_t b);
};

#endif // TINY_RLNC_H