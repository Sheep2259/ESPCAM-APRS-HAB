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
#define XCLK_FREQ_MHZ 20

// -------------------------------------------------------
// StartCamera()
// Initialises the camera hardware.
// Call once from setup(). Halts with Serial errors on failure.
// -------------------------------------------------------
esp_err_t StartCamera() {
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
    config.frame_size   = FRAMESIZE_SVGA;   // change as needed (see below)
    config.jpeg_quality = 8;               // 0–63; lower = higher quality / larger file
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.fb_count     = 2;               // double-buffer; use 1 if no PSRAM
    config.grab_mode    = CAMERA_GRAB_LATEST;

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


    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("CRITICAL: Camera init failed with error 0x%x\n", err);
        Serial.println("A full power cycle (off/on) may be needed to recover.");
        return err;
    }

    // Optional: tweak sensor defaults after init
    sensor_t *s = esp_camera_sensor_get();


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


    Serial.println("Camera init succeeded");
    return err;
}



void resetCamera() {
  pinMode(PWDN_GPIO_NUM, OUTPUT);
  
  // Pull PWDN high to power down the camera
  digitalWrite(PWDN_GPIO_NUM, HIGH);
  delay(500);
  
  // Pull PWDN low to power up the camera and reset state
  digitalWrite(PWDN_GPIO_NUM, LOW);
  delay(500);
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
    camera_fb_t *fb = nullptr;

    for (int cycle = 0; cycle < 5; cycle++) {
        // Attempt to grab a frame, 2 retries per cycle
        for (int attempt = 0; attempt < 2; attempt++) {
            fb = esp_camera_fb_get();
            if (fb) break;
            Serial.printf("captureJpeg: cycle %d, attempt %d failed, retrying...\n", cycle + 1, attempt + 1);
            delay(200);
        }

        if (fb) break; // Got a frame, no need to reinit

        // No frame after retries — reinitialise the camera
        Serial.printf("captureJpeg: cycle %d failed, reinitialising camera...\n", cycle + 1);
        esp_camera_deinit();
        resetCamera();

        int warmupFrames = 0;
        if (StartCamera() == ESP_OK) {
            // Warm-up sequence after reinit
            for (int w = 0; w < 15 && warmupFrames < 3; w++) {
                camera_fb_t *warmup = esp_camera_fb_get();
                if (warmup) {
                    esp_camera_fb_return(warmup);
                    warmupFrames++;
                }
                delay(200);
            }
            Serial.println("captureJpeg: camera reinitialised.");
        } else {
            Serial.println("captureJpeg: camera reinit failed.");
        }
    }

    if (!fb) {
        Serial.printf("captureJpeg failed — heap free: %u, PSRAM free: %u\n",
                      ESP.getFreeHeap(), ESP.getFreePsram());
        Serial.println("captureJpeg: failed to acquire frame after all cycles.");
        return nullptr;
    }

    if (fb->format != PIXFORMAT_JPEG) {
        Serial.println("captureJpeg: unexpected non-JPEG frame");
        esp_camera_fb_return(fb);
        return nullptr;
    }

    return fb;
}


esp_err_t savePhoto(uint8_t quality, double lat, double lng, float alt, const char* timeStr) {
    camera_fb_t *fb = captureJpeg();
    if (!fb) {
        Serial.println("savePhoto: frame buffer could not be acquired");
        return ESP_FAIL;
    }

    uint8_t *buf = fb->buf;
    size_t len = fb->len;

    // ------------------------------------------------------------------
    // Step 2: Validate the JPEG in PSRAM before touching the filesystem.
    // A bad frame here means we never waste a flash slot on corrupt data.
    // ------------------------------------------------------------------
    if (!validateJpegBuffer(buf, len)) {
        Serial.println("savePhoto: captured frame failed JPEG validation — discarding");
        esp_camera_fb_return(fb);
        return ESP_FAIL;
    }

    uint8_t startIdx = (quality == 1) ? 0 : 8;
    uint8_t endIdx   = (quality == 1) ? 8 : 16;
    bool saved = false;

    for (uint8_t i = startIdx; i < endIdx; i++) {
        if (savedImages[i] == 0) {
            char filename[12];
            snprintf(filename, sizeof(filename), "/%d.jpg", i);

            File file = LittleFS.open(filename, FILE_WRITE);
            if (!file) {
                Serial.println("savePhoto: failed to open file for writing");
                esp_camera_fb_return(fb);
                return ESP_FAIL;
            }

            // ----------------------------------------------------------
            // Step 3: Write JPEG from PSRAM to flash in 512-byte chunks.
            // ----------------------------------------------------------
            const size_t CHUNK = 512;
            uint8_t chunkBuf[CHUNK];
            size_t totalWritten = 0;
            size_t offset = 0;
            bool writeError = false;

            while (offset < len) {
                size_t toWrite = min(CHUNK, len - offset);
                memcpy(chunkBuf, buf + offset, toWrite);   // PSRAM -> SRAM
                size_t written = file.write(chunkBuf, toWrite);
                if (written != toWrite) {
                    Serial.printf("savePhoto: write failed at offset %u "
                                  "(wanted %u got %u)\n",
                                  (unsigned)offset, (unsigned)toWrite, (unsigned)written);
                    writeError = true;
                    break;
                }
                totalWritten += written;
                offset += toWrite;
            }

            if (writeError || totalWritten != len) {
                Serial.printf("savePhoto: incomplete write (%u/%u bytes) — deleting slot\n",
                              (unsigned)totalWritten, (unsigned)len);
                file.close();
                LittleFS.remove(filename);
                esp_camera_fb_return(fb);
                return ESP_FAIL;
            }

            // ----------------------------------------------------------
            // Step 4: Append metadata trailer (original format).
            // ----------------------------------------------------------
            file.printf("||META:T=%s,LT=%.6f,LN=%.6f,A=%.0f", timeStr, lat, lng, alt);
            file.flush();
            size_t finalSize = file.size();
            file.close();

            Serial.printf("savePhoto: wrote %u bytes (JPEG) + trailer = %u total\n",
                          (unsigned)len, (unsigned)finalSize);

            if (finalSize == 0) {
                Serial.println("savePhoto: file size is 0 after write — aborting");
                LittleFS.remove(filename);
                esp_camera_fb_return(fb);
                return ESP_FAIL;
            }

            // ----------------------------------------------------------
            // Step 5: Re-read the file from flash and validate it.
            // This catches filesystem write errors or silent corruption.
            // ----------------------------------------------------------
            if (!validateJpegFile(filename)) {
                Serial.printf("savePhoto: post-write validation FAILED for %s — deleting\n",
                              filename);
                LittleFS.remove(filename);
                esp_camera_fb_return(fb);
                return ESP_FAIL;
            }

            Serial.printf("savePhoto: slot %d validated OK\n", i);

            savedImages[i] = (uint16_t)ceil((finalSize * 1.2) / 53.0);
            imageVersion[i]++;
            prefs.putBytes("version", imageVersion, sizeof(imageVersion));
            prefs.putBytes("remain", savedImages, sizeof(savedImages));
            saved = true;
            break;
        }
    }

    esp_camera_fb_return(fb);
    if (!saved) Serial.println("savePhoto: no available slots.");
    return saved ? ESP_OK : ESP_FAIL;
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

// how many images stored?
uint8_t countStoredImages(const uint16_t* savedImages) {
    uint8_t count = 0;
    for (int i = 0; i < 16; i++) {
        if (savedImages[i] != 0) count++;
    }
    return count;
}

// -------------------------------------------------------
// validateJpegBuffer()
// Checks that a buffer in RAM is a well-formed JPEG:
//   • Starts with SOI marker  FF D8
//   • Contains an EOI marker  FF D9 within the last 32 bytes
//
// Call this on the raw camera framebuffer before writing to flash.
// Returns true if the JPEG looks intact.
// -------------------------------------------------------
bool validateJpegBuffer(const uint8_t* buf, size_t len) {
    if (len < 4) {
        Serial.printf("validateJpegBuffer: too small (%u bytes)\n", (unsigned)len);
        return false;
    }
    // JPEG SOI
    if (buf[0] != 0xFF || buf[1] != 0xD8) {
        Serial.printf("validateJpegBuffer: bad SOI bytes: %02X %02X\n", buf[0], buf[1]);
        return false;
    }
    // JPEG EOI must appear somewhere in the last 32 bytes
    size_t scanFrom = (len >= 32) ? len - 32 : 0;
    for (size_t i = scanFrom; i < len - 1; i++) {
        if (buf[i] == 0xFF && buf[i + 1] == 0xD9) return true;
    }
    Serial.println("validateJpegBuffer: no EOI marker found");
    return false;
}

// -------------------------------------------------------
// validateJpegFile()
// Re-reads a saved file from LittleFS and checks:
//   • Starts with SOI  FF D8
//   • Contains an EOI  FF D9 somewhere in the final 300 bytes
//     (before the appended ||META: block)
//
// The tail scan covers the last 300 bytes of the file, which
// is well beyond the ~80-byte metadata trailer.
// Returns true if the file appears to be a valid JPEG.
// -------------------------------------------------------
bool validateJpegFile(const char* filename) {
    File f = LittleFS.open(filename, "r");
    if (!f) {
        Serial.printf("validateJpegFile: cannot open %s\n", filename);
        return false;
    }
    size_t fileSize = f.size();
    if (fileSize < 4) {
        Serial.printf("validateJpegFile: file too small (%u bytes)\n", (unsigned)fileSize);
        f.close();
        return false;
    }

    // --- Check SOI at start ---
    uint8_t header[3] = {0};
    if (f.read(header, 3) != 3 || header[0] != 0xFF || header[1] != 0xD8) {
        Serial.printf("validateJpegFile: bad SOI in %s (%02X %02X)\n",
                      filename, header[0], header[1]);
        f.close();
        return false;
    }

    // --- Scan tail for EOI ---
    // Read up to 300 bytes from near the end.
    // This sits past any real JPEG data and before/within the metadata trailer.
    const size_t TAIL_WINDOW = 200;
    size_t scanFrom = (fileSize > TAIL_WINDOW) ? fileSize - TAIL_WINDOW : 3;
    if (!f.seek(scanFrom)) {
        Serial.printf("validateJpegFile: seek failed in %s\n", filename);
        f.close();
        return false;
    }

    uint8_t prev = 0;
    bool foundEOI = false;
    while (f.available()) {
        uint8_t b = (uint8_t)f.read();
        if (prev == 0xFF && b == 0xD9) {
            foundEOI = true;
            break;
        }
        prev = b;
    }

    f.close();
    if (!foundEOI) {
        Serial.printf("validateJpegFile: no EOI marker in tail of %s\n", filename);
    }
    return foundEOI;
}
