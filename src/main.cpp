#include <RadioLib.h>
#include <Arduino.h>
//#include <SPI.h>
#include <radio.h>
#include <pin_defs.h>
#include <TinyGPSPlus.h>
#include <GPS.h>
#include <globals.h>
#include <base91.h>
#include <geofence.h>
#include <camera.h>
#include <quality.h>
#include <LinearErasureCoder.h>



// send a telemetry packet every x packets (alternating lora and 2m)
const unsigned telempacketinterval = 20;

// send a packet every 10 seconds (10000ms)
const unsigned long TX_INTERVAL = 10000;

// take and store an image every 1 hour (3600000ms)
const unsigned long IMG_interval  = 3600000;

// sync remaining packets to preferences (flash) every 10 mins
const unsigned updateprefs = 600000;




int solarvoltage;
uint32_t counter = 0;

float lat = 0.0f, lng = 0.0f, age_s = 3600.0f, hdop = 0.0f;
float alt = 0.0f, speed_kmh = 0.0f, course_deg = 0.0f;
uint16_t year = 0;
uint8_t month = 0, day = 0, hour = 0, minute = 0, second = 9, centisecond = 0, sats = 0;

uint16_t enc_alt = 80, enc_speed = 0, enc_hdop = 0, enc_bat = 1, enc_pv = 1;


// default aprs packet, variables change when we have more data like gps
char callsign[] = "M7CWV";
char destination[] = "APRS";
char latitudechars[] = "0000.00N";
char longitudechars[] = "00000.00E";
char messagelora[] = "placeholder";
char message2m[] = "placeholder";




unsigned long lastTxTime = 0;
unsigned long lastIMGTime = 0;
unsigned long lastprefsupdatetime = 0;

unsigned quality = 0;
unsigned lastquality = 0;

char telemmsg[68];

uint8_t packetData[54];
LinearErasureCoder coder(53); // 53 byte packets

char timestampchars[30];


void setup() {
  
  delay(3000);

  Serial.begin(115200);

  prefs.begin("img_data", false);
  size_t savedimagesize = prefs.getBytes("remain", savedImages, sizeof(savedImages));
  
  if (savedimagesize == 0) {
    memset(savedImages, 0, sizeof(savedImages));
  }

  size_t versionSize = prefs.getBytes("version", imageVersion, sizeof(imageVersion));

  if (versionSize == 0) {
    memset(imageVersion, 0, sizeof(imageVersion));
  }

  //SPI.setRX(sxMISO_pin); // MISO
  //SPI.setTX(sxMOSI_pin); // MOSI
  //SPI.setSCK(sxSCK_pin); // SCK
  //SPI.setCS(sxCS_pin);   // CSn
  //SPI.begin();

  GEOFENCE_position(lat, lng);

  analogReadResolution(12);

  //Serial2.begin(9600, SERIAL_8N1, gpsRXPin, gpsTXPin);  // 9600, 5, 4

  if(!LittleFS.begin(true)){
        return;
  }

  cam_init();
}


void loop() {

	while (Serial2.available() > 0) {
		if (gps.encode(Serial2.read())) {
			  UpdateGPSInfo(lat, lng, age_s,
              year, month, day,
              hour, minute, second, centisecond,
              alt, speed_kmh, course_deg,
              sats, hdop);
            
		}
	} 

  RXfinishedimages(savedImages); // recieve finished image packets and update remaining packets (if recieved)

  if ((millis() - lastprefsupdatetime) >= updateprefs){
    prefs.putBytes("remain", savedImages, sizeof(savedImages));
    lastprefsupdatetime = millis();
  }

  if ((millis() - lastIMGTime) >= IMG_interval){
    // dont need to check for free image slot as it will be discarded if no slot available
    snprintf(timestampchars, sizeof(timestampchars), "%d/%d/%d/%d/%d", month, day, hour, minute, second);

    if (quality == 1){
      savePhoto(1, lat, lng, alt, timestampchars);
      lastIMGTime = (millis() - (IMG_interval/2)); // wait half interval until next image
    }
    else{
      savePhoto(0, lat, lng, alt, timestampchars);
      lastIMGTime = millis(); // wait full interval until next image
    }
  }

  if ((lastquality == 0) && (quality == 1)){
    // when we enter a high quality area, take image
    snprintf(timestampchars, sizeof(timestampchars), "%d/%d/%d/%d/%d", month, day, hour, minute, second);
    savePhoto(1, lat, lng, alt, timestampchars);
    lastquality = 1; // so that it doesnt keep taking images
    lastIMGTime = (millis() - (IMG_interval/2)); // wait half interval until next image
  }


  //transmit stuff below here

  if (millis() - lastTxTime >= TX_INTERVAL) {


    aprsFormatLat(lat, latitudechars, sizeof(latitudechars));
    aprsFormatLng(lng, longitudechars, sizeof(longitudechars));
    lastquality = quality;
    quality = locationQuality(lat, lng);
    GEOFENCE_position(lat, lng);




    if ((counter % telempacketinterval) == 0){
      solarvoltage = analogRead(vsensesolar_pin);

      snprintf(telemmsg, sizeof(telemmsg), "T:%d/%d/%d/%d/%dP:%.0f/%.0f/%d/%.1fS:%d/%d", 
        month, day, hour, minute, second, alt, speed_kmh, sats, hdop, solarvoltage, counter); // about 47 chars used

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

      int filenum = IMGnToTX(savedImages);
      if (filenum == -1){
          return;
      }
      // 4 least significant bits is image number, 4 most is random to create prng seed variability when not moving
      uint8_t identifier = 0; 

      uint32_t raw_random = esp_random();
      // Mask with 0xF0 (binary 11000000) to keep only the higher 2 bits
      identifier = raw_random & 0xC0;
      identifier += (imageVersion[filenum] & 0x03) * 16; // take 2 least significant bits and put them in positions 00110000
      identifier += filenum; // 0-15, so 00001111

      float trunclat = truncParseLat(latitudechars); // returns lat/lng in precision aprs encodes it in,
      float trunclon = truncParseLng(longitudechars); // for prng seed so reciever can reconstruct

      char accessfilebuf[12];
      snprintf(accessfilebuf, sizeof(accessfilebuf), "/%d.jpg", filenum);

      if (coder.encodePacket(LittleFS, accessfilebuf, trunclat, trunclon, identifier, packetData)) {
        // bytes 0-52 are already filled (53 bytes)
        packetData[53] = identifier;

        char outputBuffer[68]; // max is 67 chars   \o_     _o/      \o_     _o/  last byte for termiantor
        encodeBase91(packetData, sizeof(packetData), outputBuffer);

        lastTxTime = millis(); // Reset timer
        transmit_2m(callsign, destination, latitudechars, longitudechars, outputBuffer);

        if (receptionlocation(lat, lng)){
          savedImages[filenum]--; // if pretty sure packet recieved, decrement remaining for that image (mostly a fallback from rx packets)
        }

        counter++;
      }
    }
  }
}








