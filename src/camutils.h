#ifndef CAMUTILS_H
#define CAMUTILS_H

#include <Arduino.h>
#include <Preferences.h>
#include "esp_camera.h"

extern Preferences prefs;
extern uint16_t savedImages[16];
extern uint8_t imageVersion[16];

// -----------------------------------------------------------------
// Error-detection helpers
// -----------------------------------------------------------------

/**
 * Validates a JPEG buffer in RAM.
 * Checks for SOI (FF D8) at byte 0 and EOI (FF D9) in the final 32 bytes.
 * Call this on the raw camera framebuffer before writing to flash.
 */
bool validateJpegBuffer(const uint8_t* buf, size_t len);

/**
 * Validates a JPEG file stored on LittleFS.
 * Checks for SOI at the start and EOI somewhere in the final 300 bytes
 * (safely past the appended ||META: trailer).
 * Returns true if the file looks like a complete, intact JPEG.
 */
bool validateJpegFile(const char* filename);

// -----------------------------------------------------------------
// Core camera functions
// -----------------------------------------------------------------
esp_err_t savePhoto(uint8_t quality, double lat, double lng, float alt, const char* timeStr);
int IMGnToTX(uint16_t savedImages[]);
esp_err_t StartCamera();
camera_fb_t* captureJpeg();
void resetCamera();
uint8_t countStoredImages(const uint16_t* savedImages);

#endif