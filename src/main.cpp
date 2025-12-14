#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>
#include <radio.h>
#include <rpi_defs.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <GPS.h>
#include <globals.h>
#include <BigNumber.h>
#include <tuple>
#include <vector>
#include <encoder.h>
#include <geofence.h>

int batvoltage;
int solarvoltage;
int counter = 0;


float lat = 0.0f, lng = 0.0f, age_s = 3600.0f, hdop = 0.0f;
float alt = 0.0f, speed_kmh = 0.0f, course_deg = 0.0f;
uint16_t year = 0;
uint8_t month = 0, day = 0, hour = 0, minute = 0, second = 9, centisecond = 0, sats = 0;


uint16_t enc_alt = 80, enc_speed = 0, enc_hdop = 0, enc_bat = 1, enc_pv = 1;



// default aprs packet, variables change when we have more data like gps
char callsign[] = "M7CWV";
char destination[] = "APRS";
char latitude[] = "0000.00N";
char longitude[] = "00000.00E";
char messagelora[] = "placeholder";
char message2m[] = "placeholder";




// structure (and bases) of the data that we feed into the mixed radix encoder, updated every gps read
std::vector<std::tuple<uint16_t, uint16_t>> digits_and_bases = {
    //{frequency, 1}, // 0 for vhf (2m), 1 for uhf (lora)
    {enc_alt, 280},
    {sats, 40},
    {enc_speed, 62},
    {enc_hdop, 270},
    {enc_bat, 410},
    {enc_pv, 410},
    {counter, 1000}
};





void setup() {
  Serial.begin(115200);
  Serial.print("test");
  
  SPI.setRX(sxMISO_pin); // MISO
  SPI.setTX(sxMOSI_pin); // MOSI
  SPI.setSCK(sxSCK_pin); // SCK
  SPI.setCS(sxCS_pin);   // CSn
  SPI.begin();

  GEOFENCE_position(lat, lng);

  analogReadResolution(12);
  
  ss.begin(GPSBaud);

  BigNumber::begin ();

}


void loop() {

  solarvoltage = analogRead(vsensesolar_pin);
  batvoltage = analogRead(vsensebat_pin);
  int uptime = millis() / 1000;


  /*
  for (int i = 0; i < 3; i++) {
    transmit_2m(callsign, destination, latitude, longitude, message2m);
    delay(100);
    transmit_2m(callsign, destination, latitude, longitude, message2m);
    delay(6000);

    counter++;
  }
  */

  unsigned long start = millis(); // start of loop time

	while (ss.available() > 0 && (millis() - start < 10)) { // give gps loop max of 1s to complete, to avoid getting stuck
		if (gps.encode(ss.read())) {
			  displayInfo(lat, lng, age_s,
              year, month, day,
              hour, minute, second, centisecond,
              alt, speed_kmh, course_deg,
              sats, hdop);
         
        aprsFormatLat(lat, latitude, sizeof(latitude));
        aprsFormatLng(lng, longitude, sizeof(longitude));

        MRencode_convert(hdop, alt, speed_kmh, course_deg, batvoltage, solarvoltage, &enc_alt, &enc_speed, &enc_hdop, &enc_bat, &enc_pv);

        std::vector<std::tuple<uint16_t, uint16_t>> digits_and_bases = {
        //{frequency, 1}, // 0 for vhf (2m), 1 for uhf (lora)
        {enc_alt, 280},
        {sats, 40},
        {enc_speed, 62},
        {enc_hdop, 270},
        {enc_bat, 410},
        {enc_pv, 410},
        {counter, 1000}
        };

        BigNumber intpayload = encodeMixedRadix(digits_and_bases);

        String base91payload = toBase91(intpayload);

        Serial.println(intpayload);
        Serial.println(base91payload);
          
        




  delay(500);
      
		}
	}  
}






