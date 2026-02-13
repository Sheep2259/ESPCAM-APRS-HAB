#include "camera.h"
#include "esp32cam.h"
#include "LittleFS.h"


esp32cam::Resolution initialResolution;
Preferences prefs;

// savedImages array contains the remaining packets required for complete transmission
// default is 0 for nonexistent images, when remaining packets are decremented to 0 it is safe to overwrite
uint16_t savedImages[16];


void updateRemaining() {
  prefs.putBytes("counts", savedImages, sizeof(savedImages));
}


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


void savePhoto(uint8_t quality) {
  // Capture the frame
  auto frame = esp32cam::capture(); // Returns std::unique_ptr<Frame>

  if (frame == nullptr) {
    Serial.println("Capture failed");
    return;
  }

  // determine index range based on quality
  // if quality is 1 (high): range 0-7
  // if quality is 0 (low): range 8-15
  uint8_t startIdx = (quality == 1) ? 0 : 8;
  uint8_t endIdx = (quality == 1) ? 8 : 16;

  for (uint8_t i = startIdx; i < endIdx; i++) {

    if (savedImages[i] == 0) { 

      // construct file name
      char filename[12];
      snprintf(filename, sizeof(filename), "/%d.jpg", i);
      
      File file = LittleFS.open(filename, FILE_WRITE);

      if (!file) {
        Serial.println("Failed to open file for writing");
        return;
      }

      file.write(frame->data(), frame->size()); 
      
      // update savedimages packet remaining count
      savedImages[i] = ceil((frame->size() * 1.1) / 50.0);

      file.close();

      updateRemaining();

      return;
    }
  }
}

