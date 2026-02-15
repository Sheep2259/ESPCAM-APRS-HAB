#include <RadioLib.h>
#include <Arduino.h>
//#include <SPI.h>
#include <radio.h>
#include <pin_defs.h>
#include <TinyGPSPlus.h>
#include <GPS.h>
#include <globals.h>
#include <BigNumber.h>
#include <tuple>
#include <vector>
#include <base91.h>
#include <geofence.h>
#include <camera.h>
#include <quality.h>
#include <LinearErasureCoder.h>

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




// initialize bases here. Values (0) are placeholders.
RadixItem payloadData[PAYLOAD_ITEMS] = {
    {0, 720},  // encoded Alt
    {0, 40},   // Sats
    {0, 62},   // encoded Speed
    {0, 270},  // encoded HDOP
    {0, 410},  // encoded PV
    {0, 1000}  // Counter
};

char base91payload[100];

unsigned long lastTxTime = 0;
const unsigned long TX_INTERVAL = 10000; // 10 seconds

unsigned long lastIMGTime = 0;
const unsigned long IMG_interval  = 3600000; // 1 hour

unsigned quality = 0;
unsigned lastquality = 0;

uint8_t packetData[53];
LinearErasureCoder coder(52); // 52 byte packets


void setup() {
  
  delay(3000);

  Serial.begin(115200);

  prefs.begin("img_data", false);
  size_t savedimagesize = prefs.getBytes("counts", savedImages, sizeof(savedImages));
  
  if (savedimagesize == 0) {
    memset(savedImages, 0, sizeof(savedImages));
  }

  //SPI.setRX(sxMISO_pin); // MISO
  //SPI.setTX(sxMOSI_pin); // MOSI
  //SPI.setSCK(sxSCK_pin); // SCK
  //SPI.setCS(sxCS_pin);   // CSn
  //SPI.begin();

  GEOFENCE_position(lat, lng);

  analogReadResolution(12);

  //Serial2.begin(9600, SERIAL_8N1, gpsRXPin, gpsTXPin);  // 9600, 5, 4

  BigNumber::begin ();

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

  if ((millis() - lastIMGTime) >= IMG_interval){
    // dont need to check for free image slot as it will be discarded if no slot available
    if (quality == 1){
      savePhoto(1);
      lastIMGTime = (millis() - (IMG_interval/2)); // wait half interval until next image
    }
    else{
      savePhoto(0);
      lastIMGTime = millis(); // wait full interval until next image
    }
  }

  if ((lastquality == 0) && (quality == 1)){
    // when we enter a high quality area, take image
    savePhoto(1);
    lastquality = 1; // so that it doesnt keep taking images
    lastIMGTime = (millis() - (IMG_interval/2)); // wait half interval until next image
  }


  //transmit stuff below here

  if (millis() - lastTxTime >= TX_INTERVAL) {

    if ((counter % 20) == 0){
      solarvoltage = analogRead(vsensesolar_pin);
      int uptime = millis() / 1000;
      lastquality = quality;
      quality = locationQuality(lat, lng);
      GEOFENCE_position(lat, lng);
            
      aprsFormatLat(lat, latitudechars, sizeof(latitudechars));
      aprsFormatLng(lng, longitudechars, sizeof(longitudechars));

      MRencode_convert(hdop, alt, speed_kmh, course_deg, solarvoltage, &enc_alt, &enc_speed, &enc_hdop, &enc_bat, &enc_pv);

      payloadData[0].value = enc_alt;
      payloadData[1].value = sats;
      payloadData[2].value = enc_speed;
      payloadData[3].value = enc_hdop;
      payloadData[4].value = enc_pv;
      payloadData[5].value = counter;

      BigNumber intpayload = encodeMixedRadix(payloadData, PAYLOAD_ITEMS);

      toBase91(intpayload, base91payload, sizeof(base91payload));

      /*
      Serial.print("Lat: "); Serial.println(lat, 6);
      Serial.print("Lng: "); Serial.println(lng, 6);
      Serial.print("Age (s): "); Serial.println(age_s, 2);
      Serial.print("Date: "); Serial.print(day); Serial.print("/"); Serial.print(month); Serial.print("/"); Serial.println(year);
      Serial.print("Time (UTC): ");
      Serial.print(hour); Serial.print(":"); Serial.print(minute); Serial.print(":"); Serial.println(second);
      Serial.print("Alt (m): "); Serial.println(alt, 2);
      Serial.print("Speed (km/h): "); Serial.println(speed_kmh, 2);
      Serial.print("Course (deg): "); Serial.println(course_deg, 2);
      Serial.print("Satellites: "); Serial.println(sats);
      Serial.print("HDOP: "); Serial.println(hdop, 2);
      Serial.print("Counter: "); Serial.println(counter);
      Serial.println();
      */

      lastTxTime = millis(); // Reset timer

      if ((counter % 40) == 0) { 
        transmit_2m(callsign, destination, latitudechars, longitudechars, base91payload);
        Serial.print(base91payload);
        Serial.println(" :2m payload");
        counter++;
      } // do half and half 2m/lora for telem packets
      else {
        transmit_lora(callsign, destination, latitudechars, longitudechars, base91payload);
        Serial.print(base91payload);
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
      // 4 least significant bits image number, 4 most is random to create prng seed variability when not moving
      uint8_t identifier = 0; 

      uint32_t raw_random = esp_random();
      // Mask with 0xF0 (binary 11110000) to keep only the higher 4 bits
      identifier = raw_random & 0xF0;
      identifier += filenum;

      aprsFormatLat(lat, latitudechars, sizeof(latitudechars));
      aprsFormatLng(lng, longitudechars, sizeof(longitudechars));

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

        counter++;
      }
    }
  }
}








