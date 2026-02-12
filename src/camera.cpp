#include "camera.h"
#include "esp32cam.h"
#include "LittleFS.h"


esp32cam::Resolution initialResolution;
Preferences prefs;
uint16_t savedImages[16];

void syncToFlash() {
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

  for (uint8_t i = 0; i < 16; i++) {

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

      syncToFlash();

      return;
    }
  }
}