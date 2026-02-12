#ifndef TINY_RLNC_H
#define TINY_RLNC_H

#include <cstdint>
#include <cstddef> // For size_t

#define RLNC_BLOCK_SIZE 50

class TinyRLNC {
public:
    /**
     * @brief Initialize with total byte size.
     * @param data_ptr Pointer to source data.
     * @param total_bytes Exact size of the file/data in bytes.
     * * The class automatically calculates how many 50-byte blocks are needed.
     * It handles padding internally (virtual zeros) if size isn't a multiple of 50.
     */
    TinyRLNC(const uint8_t* data_ptr, size_t total_bytes);

    void encode(uint32_t seed, uint8_t* out_buffer);

private:
    const uint8_t* _data_ptr;
    size_t _total_bytes;
    uint32_t _full_blocks;   // Number of blocks that are exactly 50 bytes
    uint8_t _partial_bytes;  // Bytes in the final block (if any)

    static uint32_t xorshift32(uint32_t& state);
    static inline uint8_t gf_mul(uint8_t a, uint8_t b);
};

#endif