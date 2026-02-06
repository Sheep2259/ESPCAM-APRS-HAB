#include <Arduino.h>
#include <RS-FEC.h> // Assuming simonyipeter/Arduino-FEC from registry

// CONFIGURATION
const uint8_t PACKET_SIZE = 54;  // Size of the payload per packet
const uint16_t DATA_PKTS = 1600;     // K: Number of data packets
const uint16_t FEC_PKTS = 3200;      // M: Number of parity packets (Redundancy)
const uint16_t TOTAL_PKTS = DATA_PKTS + FEC_PKTS;

// RS Object: (MsgLength, ECCLength)
// NOTE: In vertical striping, "MsgLength" is the total 'N' (Data + FEC)
// and "ECCLength" is the number of FEC packets.
RS::ReedSolomon<TOTAL_PKTS, FEC_PKTS> rs;

// Buffers
uint8_t originalData[DATA_PKTS * PACKET_SIZE]; 
uint8_t fecData[FEC_PKTS * PACKET_SIZE];

// We need a temporary buffer to hold one "column" (1 byte from each packet)
uint8_t columnBuffer[TOTAL_PKTS];
uint8_t repairedColumn[TOTAL_PKTS];

void generateFEC() {
    // Loop through every byte index (0 to PACKET_SIZE-1)
    for (int i = 0; i < PACKET_SIZE; i++) {
        
        // 1. Build the Message Column (Vertical Slice)
        // Grab the i-th byte from every Data Packet
        for (int row = 0; row < DATA_PKTS; row++) {
            columnBuffer[row] = originalData[(row * PACKET_SIZE) + i];
        }

        // 2. Encode this column
        // The library calculates the parity bytes and places them 
        // at the end of the buffer (indices DATA_PKTS to TOTAL_PKTS-1)
        rs.Encode(columnBuffer, repairedColumn);

        // 3. Extract the Parity bytes and save to FEC packets
        for (int row = 0; row < FEC_PKTS; row++) {
            // The parity data typically sits at the end of the encoded block
            // Check your specific library's "parity offset"
            fecData[(row * PACKET_SIZE) + i] = repairedColumn[DATA_PKTS + row];
        }
    }
}
/*
// Check if we can reconstruct the image
// validPackets: boolean array where true = packet received, false = lost
// receivedPool: A single flat buffer containing all received packets (Data and FEC)
//               placed in their correct "slots". Lost packets can be zeros.
void recoverPackets(bool* validPackets, uint8_t* receivedPool) {
    
    // Iterate vertically again
    for (int i = 0; i < PACKET_SIZE; i++) {
        
        // 1. Construct the column from what we have
        for (int row = 0; row < TOTAL_PKTS; row++) {
            if (validPackets[row]) {
                columnBuffer[row] = receivedPool[(row * PACKET_SIZE) + i];
            } else {
                columnBuffer[row] = 0; // Erasure placeholder
            }
        }

        // 2. Decode
        // IMPORTANT: Standard libraries like Arduino-FEC use Berlekamp-Massey.
        // They might fail if > FEC_PKTS/2 packets are lost.
        // A true "Erasure" library would take 'validPackets' as an argument.
        if (rs.Decode(columnBuffer, repairedColumn) == 0) {
             // Decode successful, 'repairedColumn' now has the missing bytes
             
             // 3. Write recovered bytes back to the main pool
             for(int row = 0; row < DATA_PKTS; row++) {
                 if (!validPackets[row]) {
                     receivedPool[(row * PACKET_SIZE) + i] = repairedColumn[row];
                 }
             }
        } else {
            Serial.println("Too many errors/erasures in this column!");
        }
    }
}
*/