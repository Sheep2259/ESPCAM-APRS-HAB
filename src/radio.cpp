#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>
#include <rpi_defs.h>
#include <geofence.h>

  SX1278 radio = new Module(sxNSS_pin, sxDIO0_pin, sxRESET_pin, sxDIO1_pin);
  AFSKClient audio(&radio, sxDIO2_pin);
  AX25Client ax25(&audio);
  APRSClient aprs(&ax25);

  APRSClient loraaprs(&radio);



void transmit_2m(char callsign[], char destination[], char latitude[], char longitude[], char message[]){

  if (GEOFENCE_no_tx){ // dont do anything if tx disallowed
    return;
  }
  // initialize SX1278 for 2m APRS
  int beginfskstate = radio.beginFSK(GEOFENCE_2mAPRS_frequency / 1000000);

  // initialize AX.25 client
  // source station SSID:         0
  // preamble length:             8 bytes
  int beginax25state = ax25.begin(callsign);
  

  // initialize APRS client
  // symbol: '>' (car)
  int aprsbeginstate = aprs.begin('>');

  int sendstate = aprs.sendPosition(destination, 0, latitude, longitude, message);

  // Serial.println(F("[APRS] All done!"));
}


void transmit_lora(char callsign[], char destination[], char latitude[], char longitude[], char message[]){

    if (GEOFENCE_no_tx){ // dont do anything if tx disallowed
    return;
  }

  // initialize SX1278 with the settings necessary for LoRa iGates
  Serial.print(F("[SX1278] Initializing ... "));
  // frequency:                   433.775 MHz
  // bandwidth:                   125 kHz
  // spreading factor:            9
  // coding rate:                 4/7
  int radiobeginstate = radio.begin(433.775, 125, 9, 7);

  // initialize APRS client
  Serial.print(F("[APRS] Initializing ... "));
  // symbol:                      '>' (car)
  // callsign                     "N7LEM"
  // SSID                         1
  int loraaprsstate = loraaprs.begin('>', callsign, 1);

  // SSID is set to 1, as APRS over LoRa uses WIDE1-1 path by default
  int state = loraaprs.sendPosition(destination, 1, latitude, longitude, message);
}