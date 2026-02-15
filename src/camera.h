#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include <Preferences.h>

// Declare globals as extern (tells main.cpp they exist in camera.cpp)
extern Preferences prefs;
extern uint16_t savedImages[16];

// Function prototypes
void cam_init();
void savePhoto(uint8_t quality);
void updateRemaining();
int IMGnToTX(uint16_t savedImages[]);

#endif