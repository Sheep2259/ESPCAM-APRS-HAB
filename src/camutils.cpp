#include <LittleFS.h>
#include "esp_camera.h"
#include <Preferences.h>
#include <math.h>
#include <camutils.h>

Preferences prefs;

// savedImages array contains the remaining packets required for complete transmission
// default is 0 for nonexistent images, when remaining packets are decremented to 0 it is safe to overwrite
uint16_t savedImages[16];

uint8_t imageVersion[16];


#include <esp_camera.h>




#define CAMERA_MODEL_AI_THINKER


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

// -------------------------------------------------------
// Camera clock speed in MHz.
// 8 MHz is conservative and avoids artefacts on clones.
// Increase to 20 for best frame rate if your module handles it.
// -------------------------------------------------------
#define XCLK_FREQ_MHZ 8

// -------------------------------------------------------
// StartCamera()
// Initialises the camera hardware.
// Call once from setup(). Halts with Serial errors on failure.
// -------------------------------------------------------
void StartCamera() {
    camera_config_t config;

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = Y2_GPIO_NUM;
    config.pin_d1       = Y3_GPIO_NUM;
    config.pin_d2       = Y4_GPIO_NUM;
    config.pin_d3       = Y5_GPIO_NUM;
    config.pin_d4       = Y6_GPIO_NUM;
    config.pin_d5       = Y7_GPIO_NUM;
    config.pin_d6       = Y8_GPIO_NUM;
    config.pin_d7       = Y9_GPIO_NUM;
    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn     = PWDN_GPIO_NUM;
    config.pin_reset    = RESET_GPIO_NUM;

    config.xclk_freq_hz = XCLK_FREQ_MHZ * 1000000;
    config.pixel_format = PIXFORMAT_JPEG;   // must be JPEG for fb->buf to be usable directly
    config.frame_size   = FRAMESIZE_VGA;   // change as needed (see below)
    config.jpeg_quality = 20;               // 0–63; lower = higher quality / larger file
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.fb_count     = 1;               // double-buffer; use 1 if no PSRAM
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

    /*
     * frame_size options (from API.md / esp-camera headers):
     *   FRAMESIZE_THUMB  (96x96)
     *   FRAMESIZE_QQVGA  (160x120)
     *   FRAMESIZE_HQVGA  (240x176)
     *   FRAMESIZE_QVGA   (320x240)
     *   FRAMESIZE_CIF    (400x296)
     *   FRAMESIZE_HVGA   (480x320)
     *   FRAMESIZE_VGA    (640x480)
     *   FRAMESIZE_SVGA   (800x600)   <-- default here
     *   FRAMESIZE_XGA    (1024x768)
     *   FRAMESIZE_HD     (1280x720)
     *   FRAMESIZE_SXGA   (1280x1024)
     *   FRAMESIZE_UXGA   (1600x1200)
     *   FRAMESIZE_FHD    (1920x1080) -- 3MP+ sensors only (OV3660)
     *   FRAMESIZE_QXGA   (2048x1536) -- 3MP+ sensors only
     */

#if defined(CAMERA_MODEL_ESP_EYE)
    // ESP-EYE button GPIOs need pull-ups or the camera misbehaves
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);
#endif

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("CRITICAL: Camera init failed with error 0x%x\n", err);
        Serial.println("A full power cycle (off/on) may be needed to recover.");
        while (true) {
            Serial.println("Camera init failed — halted.");
            delay(5000);
        }
    }

    Serial.println("Camera init succeeded");

    // Optional: tweak sensor defaults after init
    sensor_t *s = esp_camera_sensor_get();

    // OV3660 comes out flipped and over-saturated from the factory
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);
        s->set_brightness(s, 1);
        s->set_saturation(s, -2);
    }

    // Uncomment and adjust any of these to change default image properties:
    // s->set_framesize(s,  FRAMESIZE_VGA);
    // s->set_quality(s,    10);       // 0–63
    // s->set_brightness(s,  0);       // -2 to 2
    // s->set_contrast(s,    0);       // -2 to 2
    // s->set_saturation(s,  0);       // -2 to 2
    // s->set_hmirror(s,     0);       // 0 or 1
    // s->set_vflip(s,       0);       // 0 or 1
    // s->set_whitebal(s,    1);       // AWB: 0=off 1=on
    // s->set_awb_gain(s,    1);       // 0=off 1=on
    // s->set_exposure_ctrl(s, 1);     // AEC: 0=off 1=on
    // s->set_gain_ctrl(s,   1);       // AGC: 0=off 1=on
}

// -------------------------------------------------------
// captureJpeg()
// Grabs one JPEG frame from the camera.
//
// Returns a camera_fb_t* on success; nullptr on failure.
// YOU MUST call esp_camera_fb_return(fb) when done with
// the buffer, or memory will leak.
// -------------------------------------------------------
camera_fb_t* captureJpeg() {
    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
        Serial.printf("captureJpeg failed — heap free: %u, PSRAM free: %u\n",
                      ESP.getFreeHeap(), ESP.getFreePsram());
        Serial.println("captureJpeg: failed to acquire frame");
        return nullptr;
    }

    if (fb->format != PIXFORMAT_JPEG) {
        // This shouldn't happen if config.pixel_format = PIXFORMAT_JPEG,
        // but guard it anyway.
        Serial.println("captureJpeg: unexpected non-JPEG frame — returning buffer");
        esp_camera_fb_return(fb);
        return nullptr;
    }

    digitalWrite(33, LOW);
    return fb;
}


esp_err_t savePhoto(uint8_t quality, double lat, double lng, float alt, const char* timeStr) {
    camera_fb_t *fb = captureJpeg();
    if (!fb) {
        Serial.println("Frame buffer could not be acquired");
        return ESP_FAIL;
    }

    uint8_t *buf = fb->buf;
    size_t len = fb->len;

    uint8_t startIdx = (quality == 1) ? 0 : 8;
    uint8_t endIdx   = (quality == 1) ? 8 : 16;
    bool saved = false;

    for (uint8_t i = startIdx; i < endIdx; i++) {
        if (savedImages[i] == 0) {
            char filename[12];
            snprintf(filename, sizeof(filename), "/%d.jpg", i);

            File file = LittleFS.open(filename, FILE_WRITE);
            if (!file) {
                Serial.println("Failed to open file for writing");
                esp_camera_fb_return(fb);
                return ESP_FAIL;
            }

            // Copy from PSRAM to internal RAM in chunks before writing
            const size_t CHUNK = 512;
            uint8_t chunkBuf[CHUNK];
            size_t totalWritten = 0;
            size_t offset = 0;

            while (offset < len) {
                size_t toWrite = min(CHUNK, len - offset);
                memcpy(chunkBuf, buf + offset, toWrite);  // PSRAM -> internal RAM
                size_t written = file.write(chunkBuf, toWrite);
                if (written != toWrite) {
                    Serial.printf("Write failed at offset %u\n", offset);
                    break;
                }
                totalWritten += written;
                offset += toWrite;
            }

            Serial.printf("Wrote %u of %u bytes\n", totalWritten, len);

            file.printf("||META:T=%s,LT=%.6f,LN=%.6f,A=%.0f", timeStr, lat, lng, alt);
            file.flush();                    // force write to filesystem
            size_t finalSize = file.size(); // now accurate
            file.close();

            if (finalSize == 0) {
                Serial.println("savePhoto: file size still 0 after write");
                esp_camera_fb_return(fb);
                return ESP_FAIL;
            }

            savedImages[i] = ceil((finalSize * 1.1) / 50.0);
            imageVersion[i]++;
            prefs.putBytes("version", imageVersion, sizeof(imageVersion));
            prefs.putBytes("remain", savedImages, sizeof(savedImages));
            saved = true;
            break;
        }
    }

    esp_camera_fb_return(fb);
    if (!saved) Serial.println("No available slots to save image.");
    return saved ? ESP_OK : ESP_FAIL;
}



/*
esp_err_t savePhoto(uint8_t quality, double lat, double lng, float alt, const char* timeStr) {
  // Capture a frame
  camera_fb_t *fb = captureJpeg();
  if (!fb) {
    Serial.println("Frame buffer could not be acquired");
    return ESP_FAIL;
  }
  uint8_t *buf = fb->buf;
  size_t len = fb->len;

  uint8_t startIdx = (quality == 1) ? 0 : 8;
  uint8_t endIdx = (quality == 1) ? 8 : 16;
  bool saved = false;
  for (uint8_t i = startIdx; i < endIdx; i++) {
    if (savedImages[i] == 0) { 
      char filename[12];
      snprintf(filename, sizeof(filename), "/%d.jpg", i);
      
      File file = LittleFS.open(filename, FILE_WRITE);
      if (!file) {
        Serial.println("Failed to open file for writing");
        esp_camera_fb_return(fb);
        return ESP_FAIL;
      }
      // Write the image data
      file.write(buf, len); 
      
      // Append metadata to the end of the file
      file.printf("||META:T=%s,LT=%.6f,LN=%.6f,A=%.0f", timeStr, lat, lng, alt);
      size_t finalSize = file.size(); 
      file.close();

      size_t written = file.write(buf, len);
      Serial.printf("Attempted to write %u bytes, actually wrote %u bytes\n", len, written);

      if (finalSize == 0){
        Serial.print("image size 0, error");
        esp_camera_fb_return(fb);
        return ESP_FAIL;
      }

      savedImages[i] = ceil((finalSize * 1.1) / 50.0); 

      imageVersion[i]++;
      
      prefs.putBytes("version", imageVersion, sizeof(imageVersion));
      prefs.putBytes("remain", savedImages, sizeof(savedImages));
      saved = true;
      break;
    }
  }
  esp_camera_fb_return(fb);
  if (!saved) {
    Serial.println("No available slots to save image.");
  }
  return saved ? ESP_OK : ESP_FAIL;
}
*/




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
