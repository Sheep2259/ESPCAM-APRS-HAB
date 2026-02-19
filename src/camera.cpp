#include "camera.h"
#include "LittleFS.h"
#include "esp_camera.h"


// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22





Preferences prefs;

// savedImages array contains the remaining packets required for complete transmission
// default is 0 for nonexistent images, when remaining packets are decremented to 0 it is safe to overwrite
uint16_t savedImages[16];

uint8_t imageVersion[16];



void cam_init() {
  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // jpg setup
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
}




// take image with metadata
void savePhoto(uint8_t quality, double lat, double lng, float alt, const char* timeStr) {
  camera_fb_t * fb = NULL; 

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
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
        esp_camera_fb_return(fb); // memory release
        return;
      }

      // Write the image data first
      file.write(fb->buf, fb->len); 
      
      // Append metadata to the end of the file
      file.printf("||META:T=%s,LT=%.6f,LN=%.6f,A=%.0f", timeStr, lat, lng, alt);

      size_t finalSize = file.size(); 
      savedImages[i] = ceil((finalSize * 1.1) / 50.0); 

      file.close();

      imageVersion[i]++;
      
      prefs.putBytes("version", imageVersion, sizeof(imageVersion));
      prefs.putBytes("remain", savedImages, sizeof(savedImages));

      esp_camera_fb_return(fb); // memory release
      return;
    }
  }
  
  esp_camera_fb_return(fb); // memory release
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
