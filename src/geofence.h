
#ifndef GEOFENCE_H
#define GEOFENCE_H


#include "stdint.h"



// VARIABLES
extern float GEOFENCE_2mAPRS_frequency; 
extern float GEOFENCE_loraAPRS_frequency; 
extern bool GEOFENCE_no_tx;
extern unsigned GEOFENCE_loraAPRS_sf;
extern unsigned GEOFENCE_loraAPRS_cr;

// FUNCTIONS
int32_t pointInPolygonF(int32_t polyCorners, float * polygon, float latitude, float longitude);
void GEOFENCE_position(float latitude, float longitude);



#endif 
