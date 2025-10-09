#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>
#include <radio.h>
#include <rpi_defs.h>

int batvoltage;
int solarvoltage;
int counter = 0;
int seconds;




void setup() {
  Serial.begin(115200);
  Serial.print("test");
  
  SPI.setRX(sxMISO_pin); // MISO
  SPI.setTX(sxMOSI_pin); // MOSI
  SPI.setSCK(sxSCK_pin); // SCK
  SPI.setCS(sxCS_pin);   // CSn
  SPI.begin();

  analogReadResolution(12);
}


void loop() {
  char callsign[] = "M7CWV";
  char destination[] = "APRS";
  char latitude[] = "4911.67N";
  char longitude[] = "01635.96E";
  char messagelora[] = "lora transmission";
  char message2m[] = "2m transmission";

  solarvoltage = analogRead(vsensesolar_pin);
  batvoltage = analogRead(vsensebat_pin);
  int uptime = millis() / 1000;

  sprintf(messagelora, "u,S%d,B%d,T%d,C%d", solarvoltage, batvoltage, uptime, counter);
  sprintf(message2m, "v,S%d,B%d,T%d,C%d", solarvoltage, batvoltage, uptime, counter);


  // the following things must be transmitted periodically:
  // a range of packet lengths to test how many characters are usable,
  // packets with a wide veriaty of characters so we can tell which ones can be used for encoding,
  // packets in clusters, as in many consecutive transmission in a row to test large data thoughputs

  // Mixed radix encoding should probably be used, same as big number encoding on traquito wspr

  // aprs may have a comment message limit of 67 chars, although not certain
  // i think aprs has a min 5s between packets
  // so possibly test alternating callsign to bypass

  for (int i = 0; i < 3; i++) {
    transmit_2m(callsign, destination, latitude, longitude, message2m);
    delay(100);
    transmit_2m(callsign, destination, latitude, longitude, message2m);
    delay(6000);

    counter++;
  }
}

