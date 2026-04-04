#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>
#include <radio.h>
#include <pin_defs.h>
#include <TinyGPSPlus.h>
#include <GPS.h>
#include <base91.h>
#include <geofence.h>
#include <quality.h>
#include <LinearErasureCoder.h>
#include "camutils.h"



// send a telemetry packet every x packets (alternating lora and 2m)
const unsigned telempacketinterval = 20;

// send a packet every 10 seconds (10000ms)
const unsigned long TX_INTERVAL = 10000;

// base image store interval, 4 hours (14400000ms), subject to location
const unsigned long IMG_interval  = 14400000;

// sync remaining packets to preferences (flash) every 10 mins
const unsigned updateprefs = 600000;



uint32_t counter = 0;

float lat = 0.0f, lng = 0.0f, age_s = 3600.0f, hdop = 26.0f;
float alt = 0.0f, speed_kmh = 0.0f, course_deg = 0.0f;
uint16_t year = 0;
uint8_t month = 0, day = 0, hour = 0, minute = 0, second = 9, centisecond = 0, sats = 0;


// default aprs packet, variables change when we have more data like gps
char callsign[] = "M7CWV";
char destination[] = "APRS";
char latitudechars[] = "0000.00N";
char longitudechars[] = "00000.00E";


// debug error flags for telem
bool littlfserr = 0;
bool prefserr = 0;
bool gpserr = 1;
bool encodererr = 0;
bool camcaptureerr = 0;
bool caminiterr = 0;

uint8_t errorflags;

unsigned long lastTxTime = 0;
unsigned long lastIMGTime = 0;
unsigned long lastprefsupdatetime = 0;

unsigned quality = 0;
unsigned lastquality = 0;

char telemmsg[72];

uint8_t packetData[54];
LinearErasureCoder coder(53); // 53 byte packets

char timestampchars[30];





void setup() {
  
  Serial.begin(115200);

  delay(3000);

  Serial.print("3");
  delay(1000);
  Serial.print("2");
  delay(1000);
  Serial.print("1");
  delay(1000); 

  bool cameraReady = false;
  int initRetries = 0;

  resetCamera();

  // 1. Retry loop for initialization
  while (!cameraReady && initRetries < 20) {  
      if (StartCamera() == ESP_OK) { 
        // 2. Robust warm-up sequence
        int targetWarmupFrames = 5; 
        int successfulFrames = 0;
        int maxAttempts = 15; // Timeout threshold
        int attemptCount = 0;

        while (successfulFrames < targetWarmupFrames && attemptCount < maxAttempts) {
            camera_fb_t *warmup = esp_camera_fb_get();
            
            if (warmup) {
                esp_camera_fb_return(warmup);
                successfulFrames++;
            } else {
                Serial.println("Warm-up frame capture failed.");
            }
            
            attemptCount++;
            delay(200); 
        }

        cameraReady = true; 
        Serial.println("Camera initialized and ready.");

      } else {
          Serial.println("Camera init failed, retrying in 1s...");
          esp_camera_deinit();
          resetCamera();
          initRetries++;
      }
  }

  // Optional: Catch a total failure after 20 attempts
  if (!cameraReady) {
      Serial.println("Fatal error: Camera failed to initialize after 20 attempts.");
      caminiterr = 1;

      // ESP.restart();  not including because if would disable tracker if camera broke
  }

  
  uint8_t *frame;
  size_t   frame_len;

  prefs.begin("img_data", false);
  size_t savedimagesize = prefs.getBytes("remain", savedImages, sizeof(savedImages));
  
  if (savedimagesize == 0) {
    memset(savedImages, 0, sizeof(savedImages));
    prefs.putBytes("remain", savedImages, sizeof(savedImages));
    prefserr = 1;
  }

  size_t versionSize = prefs.getBytes("version", imageVersion, sizeof(imageVersion));

  if (versionSize == 0) {
    memset(imageVersion, 0, sizeof(imageVersion));
    prefs.putBytes("version", imageVersion, sizeof(imageVersion));
    prefserr = 1;
  }
  

  SPI.begin(sxSCK_pin, sxMISO_pin, sxMOSI_pin, -1); // NSS is fed in when the radio stuf initialiased

  GEOFENCE_position(lat, lng);

  Serial2.begin(9600, SERIAL_8N1, gpsRXPin, -1);  // 9600, 3

  if(!LittleFS.begin(true)){
    Serial.println("LittleFS mount failed!");
    littlfserr = 1;
  }
  Serial.println("LittleFS mounted OK");

  Serial.printf("LittleFS total: %u bytes\n", LittleFS.totalBytes());
  Serial.printf("LittleFS used:  %u bytes\n", LittleFS.usedBytes());
  Serial.printf("LittleFS free:  %u bytes\n", LittleFS.totalBytes() - LittleFS.usedBytes());

}


void loop() {

	while (Serial2.available() > 0) {
		if (gps.encode(Serial2.read())) {
			UpdateGPSInfo(lat, lng, age_s,
              year, month, day,
              hour, minute, second, centisecond,
              alt, speed_kmh, course_deg,
              sats, hdop);

      if (hdop < 20){
        gpserr = 0;
      }     
		}
	} 
  

  if ((millis() - lastprefsupdatetime) >= updateprefs){
    prefs.putBytes("remain", savedImages, sizeof(savedImages));
    lastprefsupdatetime = millis();
  }


  if ((millis() - lastIMGTime) >= IMG_interval){
    // dont need to check for free image slot as it will be discarded if no slot available
    snprintf(timestampchars, sizeof(timestampchars), "%d/%d/%d/%d/%d", month, day, hour, minute, second);
    quality = locationQuality(lat, lng);

    if (quality == 1){
      if (savePhoto(1, lat, lng, alt, timestampchars) == ESP_OK){
        camcaptureerr = 0;
        if (receptionlocation(lat, lng)){
          lastIMGTime = (millis() - ((IMG_interval * 3) / 4 )); // wait 1/4 interval until next image 
        }
        else{
          lastIMGTime = (millis() - (IMG_interval / 2)); // wait 1/2 interval until next image 
        }
      }
      else{
        lastIMGTime = (millis() - ((IMG_interval * 31) / 32)); // capture failed, so try again in 7.5m
        camcaptureerr = 1;
      }
    }
    else{
      if (savePhoto(0, lat, lng, alt, timestampchars) == ESP_OK){
        camcaptureerr = 0;
        if (receptionlocation(lat, lng)){
          lastIMGTime = (millis() - (IMG_interval / 2)); // wait 1/2 interval until next image
        }
        else{
          lastIMGTime = (millis() - (IMG_interval)); // wait full interval until next image 
        }
      }
      else{
        lastIMGTime = (millis() - ((IMG_interval * 31) / 32 )); // capture failed, so try again in 7.5m
        camcaptureerr = 1;
      }
    }
  }
  

  if ((lastquality == 0) && (quality == 1) && (millis() > 120000) && (hdop < 10)){
    // when we enter a high quality area, take image
    snprintf(timestampchars, sizeof(timestampchars), "%d/%d/%d/%d/%d", month, day, hour, minute, second);
    
    if (savePhoto(1, lat, lng, alt, timestampchars) == ESP_OK){
      Serial.printf("savePhoto OK");
      camcaptureerr = 0;

      lastquality = 1; // so that it doesnt keep taking images
      if (receptionlocation(lat, lng)){
        lastIMGTime = (millis() - ((IMG_interval * 3) / 4 )); // wait 1/4 interval until next image 
      }
      else{
        lastIMGTime = (millis() - (IMG_interval / 2)); // wait half interval until next image 
      }
    }
    else{
      lastIMGTime = (millis() - ((IMG_interval * 31) / 32 )); // capture failed, so try again in 7.5m
      camcaptureerr = 1; // capture failed, so try again in 7.5m
    }

    
    // Dump savedImages so you can see if any slot got written
    for (int i = 0; i < 16; i++) {
        Serial.printf("savedImages[%d] = %d\n", i, savedImages[i]);
    }
  }


  //transmit stuff below here

  if (millis() - lastTxTime >= TX_INTERVAL) {

    lastTxTime = millis();


    aprsFormatLat(lat, latitudechars, sizeof(latitudechars));
    aprsFormatLng(lng, longitudechars, sizeof(longitudechars));
    GEOFENCE_position(lat, lng);




    if ((counter % telempacketinterval) == 0){
      errorflags = packBools(gpserr, caminiterr, camcaptureerr, littlfserr, prefserr, encodererr, 0, 0);

      snprintf(telemmsg, sizeof(telemmsg), "T%d/%d/%d/%d/%dP%.0f/%.0f/%d/%.1fS%dE%02X", 
        month, day, hour, minute, second, alt, speed_kmh, sats, hdop, counter, errorflags); // about 42 chars used

      lastTxTime = millis(); // Reset timer

      if ((counter % (telempacketinterval * 2)) == 0) { // do half and half 2m/lora for telem packets
        transmit_2m(callsign, destination, latitudechars, longitudechars, telemmsg);
        Serial.print(telemmsg);
        Serial.println(" :2m payload");
        counter++;
      } 
      else {
        transmit_lora(callsign, destination, latitudechars, longitudechars, telemmsg);
        Serial.print(telemmsg);
        Serial.println(" :lora payload");
        counter++;
      }
    }

    else {
      //transmit image packet

      if (hdop > 20){
        gpserr = 1;
        counter++; // only telem if no gps
        lastTxTime = millis(); // dont want to send telem every 10s if cam broken, no point
        return;
      }

      int filenum = IMGnToTX(savedImages);
      if (filenum == -1){
        Serial.println("no packets to send");
        return;
      }
      // 4 least significant bits is image number, 4 most is random to create prng seed variability when not moving
      uint8_t identifier = 0; 

      uint32_t raw_random = esp_random();
      // Mask with 0xC0 (binary 11000000) to keep only the higher 2 bits
      identifier = raw_random & 0xC0;
      identifier += (imageVersion[filenum] & 0x03) * 16; // take 2 least significant bits and put them in positions 00110000
      identifier += filenum; // 0-15, so 00001111

      char accessfilebuf[12];
      snprintf(accessfilebuf, sizeof(accessfilebuf), "/%d.jpg", filenum);

      if (coder.encodePacket(LittleFS, accessfilebuf, latitudechars, longitudechars, identifier, packetData)) {

        // bytes 0-52 are already filled (53 bytes)
        packetData[53] = identifier;

        char outputBuffer[72]; // max is 67 chars   \o_     _o/      \o_     _o/  with a little extra
        encodeBase91(packetData, sizeof(packetData), outputBuffer);

        lastTxTime = millis(); // Reset timer
        transmit_2m(callsign, destination, latitudechars, longitudechars, outputBuffer);

        Serial.printf("%s, %s, %s", latitudechars, longitudechars, outputBuffer);
        Serial.println();

        if (receptionlocation(lat, lng)){
        savedImages[filenum]--; // if pretty sure packet recieved, decrement remaining for that image
        }

        if (savedImages[filenum] == 0){
          LittleFS.remove(accessfilebuf);
          }

        counter++;
      }
      else{
        Serial.print("encode failed");
        Serial.println(filenum);
        encodererr = 1;
        lastTxTime = millis();
        counter++; // so if critically broken it can still do telem
      }
    }
  }
}