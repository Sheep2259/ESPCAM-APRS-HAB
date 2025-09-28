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

  sprintf(messagelora, "lora,PV:%d,BV:%d,UP:%d,C:%d", solarvoltage, batvoltage, uptime, counter);
  sprintf(message2m, "2m,PV:%d,BV:%d,UP:%d,C:%d", solarvoltage, batvoltage, uptime, counter);

  for (int i = 0; i < 1; i++) {
    transmit_2m(callsign, destination, latitude, longitude, message2m);
    delay(6000);

    transmit_lora(callsign, destination, latitude, longitude, messagelora);
    delay(6000);

    counter++;
  }
}

