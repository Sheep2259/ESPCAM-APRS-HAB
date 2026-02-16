#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>
#include <pin_defs.h>
#include <geofence.h>
#include "radio.h"

  SX1278 radio = new Module(sxNSS_pin, sxDIO0_pin, sxRESET_pin, sxDIO1_pin);
  AFSKClient audio(&radio, sxDIO2_pin);
  AX25Client ax25(&audio);
  APRSClient aprs(&ax25);

  APRSClient loraaprs(&radio);


void transmit_2m(char callsign[], char destination[], char latitude[], char longitude[], char message[]){

  if (GEOFENCE_no_tx){ // dont do anything if tx disallowed
    return;
  }
  // initialize SX1278 for 2m APRS
  int beginfskstate = radio.beginFSK(GEOFENCE_2mAPRS_frequency);

  // initialize AX.25 client
  // source station SSID:         0
  // preamble length:             8 bytes
  int beginax25state = ax25.begin(callsign);
  

  // initialize APRS client
  // symbol: '>' (car)
  int aprsbeginstate = aprs.begin('>');

  int sendstate = aprs.sendPosition(destination, 0, latitude, longitude, message);

  setup_lora_rx(); // back to listening
}


void transmit_lora(char callsign[], char destination[], char latitude[], char longitude[], char message[]){

    if (GEOFENCE_no_tx){ // dont do anything if tx disallowed
    return;
  }

  // initialize SX1278 with the settings necessary for LoRa iGates
  // frequency:                   433.775 MHz
  // bandwidth:                   125 kHz
  // spreading factor:            12
  // coding rate:                 4/5
  int radiobeginstate = radio.begin(GEOFENCE_loraAPRS_frequency, 125, GEOFENCE_loraAPRS_sf, GEOFENCE_loraAPRS_cr);

  // initialize APRS client
  // symbol:                      '>' (car)
  // callsign                     "N7CWV"
  // SSID                         1
  int loraaprsstate = loraaprs.begin('>', callsign, 1);

  // SSID is set to 1, as APRS over LoRa uses WIDE1-1 path by default
  int state = loraaprs.sendPosition(destination, 1, latitude, longitude, message);

  setup_lora_rx(); // back to listening
}


void setup_lora_rx() {
  // 1. Initialize Radio in LoRa mode using your existing constants
  int state = radio.begin(GEOFENCE_loraAPRS_frequency, 125, GEOFENCE_loraAPRS_sf, GEOFENCE_loraAPRS_cr);

  if (state == RADIOLIB_ERR_NONE) {
    // 2. Start listening. 
    // This configures the SX1278 to raise DIO0 high when a packet arrives.
    radio.startReceive();
    // Serial.println(F("[LoRa] Listening..."));
  }
}


void RXfinishedimages(uint16_t* savedImages) {
  // POLL: Check if DIO0 is HIGH (Packet Received)
  if (digitalRead(sxDIO0_pin) == HIGH) {
    
    String packetData;
    // Reading the data automatically clears the DIO0 flag
    int state = radio.readData(packetData);

    if (state == RADIOLIB_ERR_NONE) {
      // Pass the raw packet. The processor handles the parsing logic.
      if (processRXpacket(packetData, savedImages)) {
        Serial.println(F("[RX] Image list updated."));
      }
    } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
      Serial.println(F("[LoRa] CRC Error"));
    }

    // Always return to listening state immediately after processing
    radio.startReceive();
  }
}

bool processRXpacket(String message, uint16_t* savedImages) {
  
  String magicSuffix = "a!c+e90@";

  // 1. Validate Suffix
  if (message.length() <= 8 || !message.endsWith(magicSuffix)) {
    return false; 
  }

  // 2. Isolate Payload
  // Remove the suffix first
  String data = message.substring(0, message.length() - 8);

  // FIX: Handle APRS Headers
  // If the packet contains ':', the message is everything AFTER the last ':'
  // APRS Format: "CALLSIGN>PATH:Message"
  int headerIndex = data.lastIndexOf(':'); 
  if (headerIndex != -1) {
    // Keep only the part after the colon
    data = data.substring(headerIndex + 1);
  }

  // Trim whitespace just in case
  data.trim(); 

  // 3. Validate Start Character
  // Now we expect the string to look like "|0|4|12|"
  if (data.length() == 0 || data.charAt(0) != '|') {
    return false;
  }

  // 4. Parse the pipes
  int currentPos = 0;
  
  while (true) {
    int nextPipe = data.indexOf('|', currentPos + 1);
    
    if (nextPipe == -1) break;

    String numStr = data.substring(currentPos + 1, nextPipe);
    
    // FIX: Only process if we actually have digits (prevents "||" causing 0 index error)
    if (numStr.length() > 0) {
      int indexToZero = numStr.toInt();

      // Bounds check
      if (indexToZero >= 0 && indexToZero < 16) {
         savedImages[indexToZero] = 0;
      }
    }

    currentPos = nextPipe;
  }

  return true;
}


// zones where reception is near garanteed

static float America[] = {
    -123.89276, 48.33711,
    -94.80097, 48.86023,
    -81.26581, 42.93535,
    -68.25800, 47.45320,
    -61.40891, 45.45535,
    -70.81320, 42.41842,
    -81.36008, 31.72237,
    -80.04172, 25.71459,
    -81.09641, 25.31800,
    -83.82101, 29.75846,
    -94.98312, 29.29961,
    -97.35617, 26.26760,
    -106.40891, 31.94638,
    -118.36203, 32.46694,
    -123.89276, 48.33711
};

static float Continental_EU[] = {
    -5.73655, 35.98252,
    -2.22092, 36.90165,
    3.84353, 43.02479,
    9.29275, 44.10737,
    15.62087, 37.94860,
    18.78494, 39.79594,
    23.97048, 35.12447,
    27.92556, 35.91137,
    28.54079, 45.41779,
    22.65212, 48.41832,
    23.35525, 54.17856,
    9.20486, 54.79123,
    -3.97874, 48.35996,
    -1.69358, 43.79098,
    -9.25217, 43.02479,
    -8.81272, 37.32219,
    -5.73655, 35.98252
};

static float Scandinavia[] = {
    9.24589, 55.04837,
    14.25566, 54.89704,
    18.82597, 59.86265,
    6.25761, 62.44751,
    4.67558, 59.95079,
    7.66386, 58.05017,
    9.24589, 55.04837
};

static float UK[] = {
    1.25812, 51.24762,
    1.43390, 52.68252,
    -2.91669, 56.52266,
    -5.86102, 56.01018,
    -3.97137, 53.65715,
    -5.59735, 50.04953,
    0.02765, 50.69417,
    1.25812, 51.24762
};

static float Japan[] = {
    130.50675, 30.97431,
    140.48829, 35.28322,
    141.93849, 40.78213,
    139.74122, 41.08094,
    138.81837, 38.06705,
    129.54591, 33.54315,
    130.50675, 30.97431
};

static float China[] = {
    121.03638, 32.67244,
    118.39966, 32.41312,
    119.45435, 26.76106,
    121.95923, 29.51205,
    121.03638, 32.67244
};

static float Am2[] = {
    -124.10579, 47.78299,
    -118.56868, 32.73103,
    -124.89680, 40.07734,
    -124.10579, 47.78299
};

bool reception = 0;

bool receptionlocation(float latitude, float longitude)
{
    // --- PRIMARY ZONES (Specific) ---
    if(pointInPolygonF(15, America, latitude, longitude) == 1)
    {
        reception = 1;
    }
    else if(pointInPolygonF(17, Continental_EU, latitude, longitude) == 1)
    {
        reception = 1;
    }
    else if(pointInPolygonF(7, Scandinavia, latitude, longitude) == 1)
    {
        reception = 1;
    }
    else if(pointInPolygonF(8, UK, latitude, longitude) == 1)
    {
        reception = 1;
    }
    else if(pointInPolygonF(7, Japan, latitude, longitude) == 1)
    {
        reception = 1;
    }
    else if(pointInPolygonF(5, China, latitude, longitude) == 1)
    {
        reception = 1;
    }
    else if(pointInPolygonF(4, Am2, latitude, longitude) == 1)
    {
        reception = 1;
    }

    // --- SECONDARY ZONES (General) ---

    else
    {
      reception = 0;
    }
  return reception;
}