void transmit_2m(char callsign[], char destination[], char latitude[], char longitude[], char message[]);
void transmit_lora(char callsign[], char destination[], char latitude[], char longitude[], char message[]);
void setup_lora_rx();
void RXfinishedimages(uint16_t* savedImages);
bool processRXpacket(String message, uint16_t* savedImages);
bool receptionlocation(float latitude, float longitude);