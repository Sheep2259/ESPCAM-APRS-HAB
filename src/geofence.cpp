/*
 * GEOFENCE.c
 *
 * Created: 5.11.2016 22:04:58
 *  Author: Tomy2
 */ 

 
#include "geofence.h"

bool GEOFENCE_no_tx = false;
float GEOFENCE_2mAPRS_frequency = 144.800;
float GEOFENCE_loraAPRS_frequency = 433.775;
unsigned GEOFENCE_loraAPRS_sf = 12;
unsigned GEOFENCE_loraAPRS_cr = 5;

/*
	Adapted version of pointInPolygon() function from:	http://alienryderflex.com/polygon/
	
	Returns '0' if the point is outside of the polygon and '1' if it's inside.
	
	Uses FLOAT input for better accuracy.
*/
int32_t pointInPolygonF(int32_t polyCorners, float * polygon, float latitude, float longitude)
{
	int32_t i;
	int32_t j = polyCorners * 2 - 2;
	int32_t oddNodes = 0;

	for(i = 0; i < polyCorners * 2; i += 2)
	{
		if((polygon[i + 1] < latitude && polygon[j + 1] >= latitude
		|| polygon[j + 1] < latitude && polygon[i + 1] >= latitude)
		&& (polygon[i] <= longitude || polygon[j] <= longitude))
		{
			oddNodes ^= (polygon[i] + (latitude - polygon[i + 1])
			/ (polygon[j + 1] - polygon[i + 1]) * (polygon[j] - polygon[i]) < longitude);
		}

		j = i;
	}

	return oddNodes;
}


/*
	Changes GEOFENCE_2mAPRS_frequency and GEOFENCE_no_tx global variables based on the input coordinates.
	
	2m FREQUENCIES:
						Africa				144.800
						Europe				144.800
						Russia				144.800
						Canada				144.390
						Mexico				144.390
						USA					144.390
						Costa Rica			145.010
						Nicaragua			145.010
						Panama				145.010
						Venezuela			145.010
						Brazil				145.570
						Colombia			144.390
						Chile				144.390
						Argentina			144.930
						Paraguay			144.930
						Uruguay				144.930
						China				144.640
						Japan				144.660
						South Korea			144.620
						Thailand			145.525
						Australia			145.175
						New Zealand			144.575
						Indonesia			144.390
						Malaysia			144.390
	
	lora FREQUENCIES:
	UK					439.9125
	Europe				433.775
	Asia				433.775
	Americas			433.775
	Indonesia/singapore 433.400
		
	Expected input FLOAT for latitude and longitude as in GPS_UBX_latitude_Float and GPS_UBX_longitude_Float.
*/

void GEOFENCE_position(float latitude, float longitude)
{
    GEOFENCE_no_tx = false;
	GEOFENCE_2mAPRS_frequency = 144.800;
	GEOFENCE_loraAPRS_frequency = 433.775;
	GEOFENCE_loraAPRS_sf = 12;
	GEOFENCE_loraAPRS_cr = 5;

    // --- PRIMARY ZONES (Specific) ---
    if(pointInPolygonF(9, ArgParUruF, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.930000;
        GEOFENCE_loraAPRS_frequency = 433.775;
    }
    else if(pointInPolygonF(7, AustraliaF, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 145.175000;
        GEOFENCE_loraAPRS_frequency = 433.775;
    }
    else if(pointInPolygonF(10, BrazilF, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 145.570000;
        GEOFENCE_loraAPRS_frequency = 433.775;
    }
    else if(pointInPolygonF(6, CostNicPanF, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 145.010000;
        GEOFENCE_loraAPRS_frequency = 433.775;
    }
    else if(pointInPolygonF(8, VenezuelaF, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 145.010000;
        GEOFENCE_loraAPRS_frequency = 433.775;
    }
    else if(pointInPolygonF(5, NewZealand, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.575;
        GEOFENCE_loraAPRS_frequency = 433.775;
    }
    else if(pointInPolygonF(10, UK, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.800000;
        GEOFENCE_loraAPRS_frequency = 439.9125;
    }
    else if(pointInPolygonF(9, Poland, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.800000;
        GEOFENCE_loraAPRS_frequency = 434.855;
		GEOFENCE_loraAPRS_sf = 9;
    }
    else if(pointInPolygonF(5, Turkmenistan, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.800000;
        GEOFENCE_loraAPRS_frequency = 434.855;
    }
    else if(pointInPolygonF(26, China, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.640000;
        GEOFENCE_loraAPRS_frequency = 433.775;
    }
    else if(pointInPolygonF(9, Japan, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.660000;
        GEOFENCE_loraAPRS_frequency = 433.775;
    }
    else if(pointInPolygonF(7, SingaporeArea, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.390;
        GEOFENCE_loraAPRS_frequency = 433.400;
    }

    // --- SECONDARY ZONES (General) ---
    else if(pointInPolygonF(7, Americas, latitude, longitude) == 1)
    {
        GEOFENCE_2mAPRS_frequency = 144.390;
    }

    else
    {
        GEOFENCE_2mAPRS_frequency = 144.800;
		GEOFENCE_loraAPRS_frequency = 433.775;
		GEOFENCE_loraAPRS_sf = 12;
    }
}