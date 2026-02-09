#include <Arduino.h>
#include "RLNC.h"

// --- Configuration ---
// 80KB Source Buffer (1600 blocks * 50 bytes)
// We allocate this globally to avoid stack overflow.
// ESP32 has plenty of RAM for this.
uint8_t sourceData[RLNC_BLOCK_COUNT * RLNC_BLOCK_SIZE]; 
uint8_t encodedBuffer1[RLNC_BLOCK_SIZE];
uint8_t encodedBuffer2[RLNC_BLOCK_SIZE];

TinyRLNC* encoder = nullptr;

// --- Helper: Fill Buffer with Pattern ---
void fillPattern(uint8_t* buffer, size_t size, uint8_t type) {
    if (type == 0) {
        memset(buffer, 0, size);
    } else if (type == 1) {
        // Sequential data
        for (size_t i = 0; i < size; i++) buffer[i] = (uint8_t)(i & 0xFF);
    } else {
        // Random-ish data
        for (size_t i = 0; i < size; i++) buffer[i] = (uint8_t)((i * 37) & 0xFF);
    }
}

// --- Test 1: Sanity Check (Zero Input) ---
// Linear combinations of zeros must result in zeros.
// If this fails, memory is being read incorrectly.
bool testSanity() {
    Serial.print("[TEST 1] Sanity (Zero Input)... ");
    
    fillPattern(sourceData, sizeof(sourceData), 0);
    encoder->encode(12345, encodedBuffer1);

    for (int i = 0; i < RLNC_BLOCK_SIZE; i++) {
        if (encodedBuffer1[i] != 0) {
            Serial.printf("FAIL at index %d (Value: %02X)\n", i, encodedBuffer1[i]);
            return false;
        }
    }
    Serial.println("PASS");
    return true;
}

// --- Test 2: Determinism ---
// Encoding with the same seed MUST yield the exact same bytes.
bool testDeterminism() {
    Serial.print("[TEST 2] Determinism (Same Seed)... ");
    
    fillPattern(sourceData, sizeof(sourceData), 2); // Random pattern
    
    uint32_t seed = 9999;
    encoder->encode(seed, encodedBuffer1);
    encoder->encode(seed, encodedBuffer2);

    if (memcmp(encodedBuffer1, encodedBuffer2, RLNC_BLOCK_SIZE) == 0) {
        Serial.println("PASS");
        return true;
    } else {
        Serial.println("FAIL (Buffers do not match)");
        return false;
    }
}

// --- Test 3: Variance ---
// Encoding with different seeds SHOULD yield different bytes.
bool testVariance() {
    Serial.print("[TEST 3] Variance (Diff Seeds)... ");
    
    fillPattern(sourceData, sizeof(sourceData), 2);
    
    encoder->encode(1001, encodedBuffer1);
    encoder->encode(1002, encodedBuffer2);

    if (memcmp(encodedBuffer1, encodedBuffer2, RLNC_BLOCK_SIZE) != 0) {
        Serial.println("PASS");
        return true;
    } else {
        Serial.println("FAIL (Buffers are identical - unexpected collision)");
        return false;
    }
}

// --- Test 4: Performance Benchmark ---
void runBenchmark() {
    Serial.println("\n[TEST 4] Performance Benchmark");
    Serial.println("------------------------------");
    
    fillPattern(sourceData, sizeof(sourceData), 2);
    
    // Warmup
    encoder->encode(1, encodedBuffer1);

    long iterations = 1000;
    unsigned long start = micros();

    for (int i = 0; i < iterations; i++) {
        // Use changing seed to prevent compiler optimization
        encoder->encode(i, encodedBuffer1);
    }

    unsigned long end = micros();
    unsigned long totalMicros = end - start;
    float avgTimeUs = (float)totalMicros / iterations;
    
    // Throughput Calculation
    // We processed the entire 80KB source block (mathematically) to produce 50 bytes.
    // However, usually throughput is measured by "Encoded Bytes Generated per Second" 
    // or "Source Data Processed per Second". 
    // Below is "Encoded Packets per Second".
    
    float packetsPerSec = 1000000.0 / avgTimeUs;
    
    Serial.printf("Iterations:      %ld\n", iterations);
    Serial.printf("Total Time:      %lu ms\n", totalMicros / 1000);
    Serial.printf("Avg Encode Time: %.2f us (%.3f ms)\n", avgTimeUs, avgTimeUs / 1000.0);
    Serial.printf("Packets/Sec:     %.2f\n", packetsPerSec);
    Serial.println("------------------------------");
}

void setup() {
    // Wait for serial monitor
    Serial.begin(115200);
    delay(2000); 
    Serial.println("\n\n=== TinyRLNC ESP32 Test Suite ===");
    Serial.printf("Source Size: %d bytes\n", sizeof(sourceData));
    Serial.printf("Block Size:  %d bytes\n", RLNC_BLOCK_SIZE);

    // Initialize Encoder
    encoder = new TinyRLNC(sourceData);

    // Run Tests
    bool allPassed = true;
    if (!testSanity()) allPassed = false;
    if (!testDeterminism()) allPassed = false;
    if (!testVariance()) allPassed = false;

    if (allPassed) {
        Serial.println("\nFunctional Tests: ALL PASSED");
        runBenchmark();
    } else {
        Serial.println("\nFunctional Tests: FAILED");
    }
}

void loop() {
    // Nothing to do here
    delay(1000);
}