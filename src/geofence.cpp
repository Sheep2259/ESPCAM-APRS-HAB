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


// GEOFENCE ARRAYS (longitude, latitude)

static float ArgParUruF[] = {
    -57.79910, -18.67750,
    -53.48140, -26.66710,
    -57.13990, -29.77390,
    -49.62520, -34.51560,
    -60.24900, -58.56250,
    -73.49850, -50.35950,
    -67.54390, -21.43260,
    -57.79910, -18.67750,
};

static float AustraliaF[] = {
    147.56840, -46.92030,
    166.02540, -29.15220,
    144.14060, -9.18890,
    98.78910, -11.69530,
    112.41210, -39.77480,
    147.56840, -46.92030,
};

static float BrazilF[] = {
    -57.04100, -29.76440,
    -49.65820, -34.45220,
    -28.30080, -5.87830,
    -51.50390, 4.30260,
    -60.55660, 5.00340,
    -74.17970, -6.49000,
    -57.74410, -18.56290,
    -53.34960, -26.66710,
    -57.04100, -29.76440,
};

static float CostNicPanF[] = {
    -88.76950, 11.99630,
    -80.20020, 4.80640,
    -76.61870, 9.42740,
    -82.70510, 15.39010,
    -88.76950, 11.99630,
};

static float VenezuelaF[] = {
    -66.65410, 0.18680,
    -60.99610, 5.41910,
    -59.52390, 9.38400,
    -72.15820, 12.49020,
    -72.48780, 7.10090,
    -67.96140, 5.72530,
    -66.65410, 0.18680,
};

static float NewZealand[] = {
    179.91211, -49.47783,
    179.64844, -31.17859,
    166.99219, -32.81830,
    157.41211, -56.25165,
    179.91211, -49.47783
};

static float Americas[] = {
    -2.32153, -57.75970,
    -112.57935, -66.19675,
    -176.30859, 51.81488,
    -162.24609, 74.76696,
    -91.47109, 82.14245,
    -48.58047, 53.12041,
    -2.32153, -57.75970
};

static float UK[] = {
    1.47031, 50.98593,
    3.32700, 53.22561,
    -0.16116, 61.63238,
    -15.84963, 58.01259,
    -6.67605, 55.35271,
    -7.99441, 54.37270,
    -5.53347, 53.96754,
    -5.68760, 52.02478,
    -8.01670, 48.95065,
    1.47031, 50.98593
};

static float Poland[] = {
    19.31229, 54.60117,
    23.53104, 54.21753,
    23.70682, 50.68478,
    22.52029, 49.32922,
    19.92752, 49.35785,
    15.26932, 51.10057,
    14.21463, 54.49921,
    18.87283, 55.40764,
    19.31229, 54.60117
};

static float Turkmenistan[] = {
    73.08814, 41.51665,
    80.20728, 42.52864,
    79.59204, 45.77504,
    72.25318, 45.43686,
    73.08814, 41.51665
};

static float China[] = {
    130.32067, 42.74266,
    134.23181, 47.57253,
    123.59704, 53.27481,
    116.96130, 49.12058,
    119.77380, 46.70592,
    106.19470, 41.73437,
    96.39346, 42.86049,
    87.56045, 48.90702,
    80.66103, 43.78582,
    80.26553, 41.98282,
    73.80557, 39.35007,
    78.81533, 34.51793,
    79.16689, 31.49667,
    88.83486, 27.87071,
    95.95400, 29.41335,
    98.52297, 27.77517,
    98.34719, 25.37802,
    97.77590, 24.10087,
    100.93996, 21.83523,
    105.51027, 23.45744,
    107.95335, 21.64772,
    107.86546, 17.80151,
    113.09140, 14.27235,
    124.32098, 24.11557,
    124.14520, 40.06482,
    130.32067, 42.74266
};

static float Japan[] = {
    124.63863, 24.19941,
    124.37496, 36.99746,
    128.85738, 38.66337,
    140.83716, 48.03792,
    150.76604, 44.51243,
    160.57617, 23.94257,
    144.05273, 11.58389,
    131.22070, 13.38587,
    124.63863, 24.19941
};

static float SingaporeArea[] = {
    92.78045, 14.90702,
    100.75212, 11.07107,
    108.88200, 3.19463,
    108.22282, -0.84665,
    100.55201, -1.88142,
    91.41138, 9.50019,
    92.78045, 14.90702
};


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
		if(((polygon[i + 1] < latitude) && (polygon[j + 1] >= latitude)
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