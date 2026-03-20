#ifndef CAMUTILS_H
#define CAMUTILS_H

#include <Arduino.h>
#include <Preferences.h>
#include "esp_camera.h"

extern Preferences prefs;
extern uint16_t savedImages[16];
extern uint8_t imageVersion[16];


esp_err_t savePhoto(uint8_t quality, double lat, double lng, float alt, const char* timeStr);
int IMGnToTX(uint16_t savedImages[]);
void StartCamera();
camera_fb_t* captureJpeg();


#endif