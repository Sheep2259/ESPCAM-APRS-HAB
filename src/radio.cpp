
#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>


void transmit_2m(){
  SX1278 radio = new Module(16, 2, 17, 5);
  AFSKClient audio(&radio, 21);
  AX25Client ax25(&audio);
  APRSClient aprs(&ax25);

  // initialize SX1278 for 2m APRS
  Serial.print(F("[SX1278] Initializing ... "));
  int beginfskstate = radio.beginFSK(144.8);

  if(beginfskstate == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(beginfskstate);
    while (true) { delay(10); }
  }

  // initialize AX.25 client
  Serial.print(F("[AX.25] Initializing ... "));
  // source station callsign:     "M7CWV"
  // source station SSID:         0
  // preamble length:             8 bytes
  int beginax25state = ax25.begin("M7CWV");
  if(beginax25state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(beginax25state);
    while (true) { delay(10); }
  }

  // initialize APRS client
  Serial.print(F("[APRS] Initializing ... "));
  // symbol:         '>' (car)
  int aprsbeginstate = aprs.begin('>');
  if(aprsbeginstate == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(aprsbeginstate);
    while (true) { delay(10); }
  }




  Serial.println(F("[APRS] Sending location reports"));
  
  // send a location with message
  char destination[] = "APRS";
  char latitude[] = "4911.67N";
  char longitude[] = "01635.96E";
  char message[] = "vscode test";
  int sendstate = aprs.sendPosition(destination, 0, latitude, longitude, message);

  Serial.println(F("[APRS] All done!"));
}







void transmit_lora(){
  SX1278 radio = new Module(16, 2, 17, 5);
  APRSClient loraaprs(&radio);


  
  // initialize SX1278 with the settings necessary for LoRa iGates
  Serial.print(F("[SX1278] Initializing ... "));
  // frequency:                   433.775 MHz
  // bandwidth:                   125 kHz
  // spreading factor:            12
  // coding rate:                 4/5
  int radiobeginstate = radio.begin(433.775, 125, 12, 5);

  // when using one of the non-LoRa modules for AX.25
  // (RF69, CC1101, Si4432 etc.), use the basic begin() method
  // int state = radio.begin();

  if(radiobeginstate == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(radiobeginstate);
    while (true) { delay(10); }
  }

  // initialize APRS client
  Serial.print(F("[APRS] Initializing ... "));
  // symbol:                      '>' (car)
  // callsign                     "N7LEM"
  // SSID                         1
  char source[] = "N7LEM";
  int loraaprsstate = loraaprs.begin('>', source, 1);
  if(loraaprsstate == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(loraaprsstate);
    while (true) { delay(10); }
  }




  Serial.print(F("[APRS] Sending position ... "));
  
  // send a location with message
  // SSID is set to 1, as APRS over LoRa uses WIDE1-1 path by default
  char destination[] = "APRS";
  char latitude[] = "4911.67N";
  char longitude[] = "01635.96E";
  char message[] = "I'm here! LORA APRS";
  int state = loraaprs.sendPosition(destination, 1, latitude, longitude, message);


  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}