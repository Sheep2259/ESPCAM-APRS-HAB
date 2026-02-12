#include "esp32cam.h"
#include "LittleFS.h"
#include <Preferences.h>

esp32cam::Resolution initialResolution;


Preferences prefs;
// savedImages array contains the remaining packets required for complete transmission
// default is 0 for nonexistent images, when remaining packets are decremented to 0 it is safe to overwrite
uint16_t savedImages[15];



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

  uint8_t numberofimages = 0;
  for (uint8_t i = 0; i < 16; i++) {

    if (savedImages[i] = 0){

      // construct file name
      char filename[12];
      snprintf(filename, sizeof(filename), "/%d.jpg", i);

      // Open a file for writing, "/i.jpg" is the filename.
      // file name structure might be 0-7 for high quality and 8-15 for low quality)
      File file = LittleFS.open(filename, FILE_WRITE);

      if (!file) {
        Serial.println("Failed to open file for writing");
        return;
      }

      // Write the buffer to the file
      // .data() gives the pointer, .size() gives the length
      file.write(frame->data(), frame->size()); 
      
      // update savedimages packet remaining count and add a 10% extra, manual ceiling operation
      savedImages[i] = (frame->size() + 49) / 50;

      // Close the file to save changes
      file.close();

      return;
    }
  }
}
