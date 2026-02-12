#include "esp32cam.h"
#include "LittleFS.h"


esp32cam::Resolution initialResolution;


void cam_init() {
  esp32cam::setLogger(Serial);

  {
    using namespace esp32cam;

    initialResolution = Resolution::find(1600, 1200);

    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(initialResolution);
    cfg.setBufferCount(1);
    cfg.setJpeg(65);

    bool ok = Camera.begin(cfg);
    if (!ok) {
      Serial.println("camera initialize failure");
      delay(5000);
      ESP.restart();
    }
    Serial.println("camera initialize success");
  }
}

void savePhoto() {
  // Capture the frame
  auto frame = esp32cam::capture(); // Returns std::unique_ptr<Frame>

  if (frame == nullptr) {
    Serial.println("Capture failed");
    return;
  }

  Serial.printf("Captured: %d bytes. Saving to LittleFS...\n", frame->size());

  // Open a file for writing
  // Note: "/photo.jpg" is the filename. Leading slash is mandatory.
  File file = LittleFS.open("/photo.jpg", FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  // Write the buffer to the file
  // .data() gives the pointer, .size() gives the length
  file.write(frame->data(), frame->size()); 
  
  // Close the file to save changes
  file.close();
  
  Serial.printf("Saved %s to flash successfully.\n", "/photo.jpg");
}
