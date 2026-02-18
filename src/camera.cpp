#include "camera.h"
#include "esp32cam.h"
#include "LittleFS.h"


esp32cam::Resolution initialResolution;
Preferences prefs;

// savedImages array contains the remaining packets required for complete transmission
// default is 0 for nonexistent images, when remaining packets are decremented to 0 it is safe to overwrite
uint16_t savedImages[16];

uint8_t imageVersion[16];



void cam_init() {
  esp32cam::setLogger(Serial);

  {
    using namespace esp32cam;

    initialResolution = Resolution::find(1600, 1200);

    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(initialResolution);
    cfg.setBufferCount(1);
    cfg.setJpeg(75);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("camera initialize failure");
      delay(5000);
      ESP.restart();
    }
    Serial.println("camera initialize success");
  }
}

// take image with metadata
void savePhoto(uint8_t quality, double lat, double lng, float alt, const char* timeStr) {
  
  auto frame = esp32cam::capture(); 

  if (frame == nullptr) {
    Serial.println("Capture failed");
    return;
  }

  uint8_t startIdx = (quality == 1) ? 0 : 8;
  uint8_t endIdx = (quality == 1) ? 8 : 16;

  for (uint8_t i = startIdx; i < endIdx; i++) {

    if (savedImages[i] == 0) { 

      char filename[12];
      snprintf(filename, sizeof(filename), "/%d.jpg", i);
      
      File file = LittleFS.open(filename, FILE_WRITE);

      if (!file) {
        Serial.println("Failed to open file for writing");
        return;
      }

      // Write the image data first
      file.write(frame->data(), frame->size()); 
      
      // Append metadata to the end of the file
      // Using a delimiter (e.g., ||META:) makes parsing easier later
      file.printf("||META:T=%s,LT=%.6f,LN=%.6f,A=%.0f", timeStr, lat, lng, alt);

      // update savedImages count based on total FILE size, not just frame size
      size_t finalSize = file.size(); 
      savedImages[i] = ceil((finalSize * 1.1) / 50.0); // 1.1x packets should be transmitted for redundancy

      file.close();

      imageVersion[i]++;
      
      prefs.putBytes("version", imageVersion, sizeof(imageVersion));
      prefs.putBytes("remain", savedImages, sizeof(savedImages));

      return;
    }
  }
}


// Returns an index (0-15) from savedImages based on weighted rank logic.
// Returns -1 if all images are 0.
int IMGnToTX(uint16_t savedImages[]) {
  float g = 0.01;

  // 1. Determine the initial range: 0 for (0-7), 1 for (8-15) (TRNG i believe)
  int rangeSelect = random(0, 2);
  
  // Struct to hold index and value for sorting
  struct ImageCandidate {
    uint8_t index;
    uint16_t value;
  };

  ImageCandidate validItems[8];
  int count = 0;

  // Helper lambda to fill candidates from a specific range
  // (Using a simple block here for Arduino compatibility)
  auto fillCandidates = [&](int range) {
    count = 0;
    int start = range * 8;
    int end = start + 8;
    for (int i = start; i < end; i++) {
      if (savedImages[i] > 0) {
        validItems[count].index = i;
        validItems[count].value = savedImages[i];
        count++;
      }
    }
  };

  // 2. Try to fill from selected range
  fillCandidates(rangeSelect);

  // 3. If no valid packets in selected range, swap to the other range
  if (count == 0) {
    rangeSelect = 1 - rangeSelect; // Flip 0 to 1 or 1 to 0
    fillCandidates(rangeSelect);
  }

  // 4. If still 0, all files are done. Return -1 (or handle as needed)
  if (count == 0) return -1;

  // 5. Sort validItems by value (Smallest to Largest)
  // Simple Bubble Sort is efficient enough for max 8 items
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      if (validItems[j].value > validItems[j + 1].value) {
        ImageCandidate temp = validItems[j];
        validItems[j] = validItems[j + 1];
        validItems[j + 1] = temp;
      }
    }
  }

  // 6. Select index using weighted formula
  // We must calculate the Base probability dynamically based on 'count'
  // so that the sum of probabilities equals 1.0.
  // Formula derived from sum: count * Base - g * (count * (count - 1) / 2) = 1.0
  
  float base = (1.0 + g * count * (count - 1) / 2.0) / (float)count;
  
  float randVal = random(0, 10001) / 10000.0; // High precision random 0.0 - 1.0
  float cumulativeProbability = 0.0;

  for (int n = 0; n < count; n++) {
    // Rank 0 (smallest packet count) gets the highest probability
    float probability = base - (g * n);
    
    cumulativeProbability += probability;

    if (randVal <= cumulativeProbability) {
      return validItems[n].index;
    }
  }

  // Fallback (in case of floating point rounding errors)
  return validItems[count - 1].index;
}
