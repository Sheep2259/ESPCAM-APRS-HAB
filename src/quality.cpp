#include "geofence.h"
#include <quality.h>



static float Paris[] = {
    3.03930, 48.43933,
    3.08324, 49.27776,
    1.49023, 49.39231,
    1.64403, 48.27142,
    3.03930, 48.43933
};

static float SE_UK[] = {
    -0.56971, 50.75093,
    0.67174, 50.79609,
    0.39709, 51.81257,
    -0.85535, 51.79898,
    -0.56971, 50.75093
};

static float Ireland_Coast[] = {
    -9.55624, 51.58176,
    -8.28183, 54.79237,
    -9.46835, 54.94411,
    -10.83065, 51.47240,
    -9.55624, 51.58176
};

static float Scotland_and_lakes[] = {
    -3.80218, 54.82336,
    -3.71429, 55.66879,
    -1.58294, 57.44838,
    -3.37371, 58.81937,
    -7.26287, 58.26900,
    -8.05818, 56.43946,
    -4.79185, 54.59248,
    -3.80218, 54.82336
};

static float Iceland[] = {
    -23.85161, 66.82267,
    -17.49049, 66.22753,
    -17.91896, 65.16971,
    -25.04909, 65.49981,
    -23.85161, 66.82267
};

static float Norway[] = {
    10.24998, 58.24546,
    6.07518, 57.33195,
    3.13084, 61.35944,
    21.36815, 71.23026,
    31.12401, 70.34836,
    21.28026, 68.70191,
    10.24998, 58.24546
};

static float Alps[] = {
    6.82225, 43.35997,
    5.48192, 45.38575,
    6.93211, 47.83803,
    12.86425, 48.18649,
    15.28125, 45.45077,
    6.82225, 43.35997
};

static float Rome[] = {
    11.60331, 42.34101,
    13.32816, 43.62685,
    14.88822, 42.06430,
    12.58109, 40.61262,
    11.60331, 42.34101
};

static float Egypt[] = {
    29.07496, 30.85778,
    30.85474, 28.96284,
    32.37085, 30.38507,
    32.56861, 31.80557,
    29.20679, 31.90821,
    29.07496, 30.85778
};

static float Israel[] = {
    34.15064, 30.82477,
    33.68921, 31.52036,
    35.23829, 33.96144,
    36.61158, 33.69677,
    35.46777, 30.56211,
    34.15064, 30.82477
};

static float Cernobyl[] = {
    30.77894, 51.12401,
    29.72700, 50.92881,
    29.40839, 51.61270,
    30.47681, 51.73194,
    30.77894, 51.12401
};

static float Ukraine[] = {
    36.66420, 50.26306,
    36.53237, 48.88103,
    31.24794, 47.13935,
    33.66494, 44.52916,
    40.01025, 48.36965,
    40.14208, 49.77359,
    36.66420, 50.26306
};

static float Dubai[] = {
    55.55727, 26.09855,
    56.96352, 25.53483,
    55.68910, 24.45948,
    54.21693, 25.40589,
    55.55727, 26.09855
};

static float Mecca[] = {
    39.80018, 21.17900,
    40.11329, 21.39398,
    39.81666, 21.72605,
    39.44312, 21.52690,
    39.80018, 21.17900
};

static float Himalayas[] = {
    81.22326, 27.49534,
    74.89514, 31.43497,
    73.79650, 35.10798,
    78.05920, 34.67544,
    79.06994, 32.07022,
    85.52990, 29.35032,
    91.90197, 28.85115,
    95.32971, 31.84651,
    99.94397, 28.77414,
    91.41858, 25.88564,
    85.61779, 26.08316,
    81.22326, 27.49534
};

static float China[] = {
    112.76770, 30.85213,
    112.85560, 27.23204,
    104.72571, 27.12454,
    104.67078, 32.05174,
    107.23060, 36.09960,
    112.76770, 30.85213
};

static float Taiwan[] = {
    118.69560, 22.55806,
    121.24443, 21.29433,
    122.82646, 25.52741,
    120.80498, 26.35726,
    118.69560, 22.55806
};

static float NK[] = {
    125.19692, 37.20636,
    129.63540, 38.66201,
    130.51431, 42.88611,
    123.74673, 39.98773,
    125.19692, 37.20636
};

static float Japan[] = {
    130.69437, 30.95773,
    141.46097, 34.90296,
    143.98542, 41.58927,
    157.64994, 49.97738,
    155.49662, 51.61598,
    146.01539, 45.52329,
    140.78589, 45.89154,
    138.14918, 39.16585,
    128.47731, 33.31993,
    130.69437, 30.95773
};

static float Pyrenees[] = {
    -1.35160, 43.66487,
    3.24068, 43.16209,
    3.14180, 41.61234,
    -2.63700, 43.07388,
    -1.35160, 43.66487
};

static float Galapagos[] = {
    -91.36230, -1.98894,
    -89.16504, -2.25243,
    -88.63770, -0.09969,
    -92.10938, 2.14102,
    -92.54883, -0.09969,
    -91.36230, -1.98894
};

static float Central_America[] = {
    -86.72183, 10.67176,
    -83.33804, 11.14643,
    -81.31655, 9.76356,
    -77.09780, 9.46026,
    -78.06460, 6.93770,
    -81.40444, 6.80681,
    -86.72183, 10.67176
};

static float Amazon[] = {
    -51.65371, 2.99518,
    -67.73769, -0.34329,
    -71.29726, -7.65925,
    -68.57265, -10.04726,
    -52.00527, -8.92032,
    -51.65371, 2.99518
};

static float Bahamas[] = {
    -79.34176, 27.34225,
    -80.66012, 23.21081,
    -72.74997, 19.65267,
    -72.31051, 22.31933,
    -79.34176, 27.34225
};

static float Canada[] = {
    -125.26185, 46.91773,
    -113.30620, 49.22583,
    -114.22905, 51.79603,
    -121.21636, 55.71565,
    -124.69639, 59.79040,
    -125.19681, 64.83477,
    -132.62357, 65.95080,
    -131.30521, 60.11396,
    -138.99564, 59.96033,
    -140.87391, 57.81330,
    -125.26185, 46.91773
};

static float Alaska[] = {
    -138.07617, 60.21128,
    -148.09570, 63.95074,
    -160.75195, 57.15275,
    -179.12109, 52.55478,
    -178.59375, 50.47688,
    -159.16992, 54.18025,
    -146.68945, 58.50959,
    -140.71289, 59.41585,
    -138.07617, 60.21128
};

static float Greenland[] = {
    -44.80246, 60.32032,
    -49.63644, 68.32943,
    -58.07394, 76.47264,
    -71.52121, 76.47264,
    -62.55637, 75.09219,
    -49.37277, 60.18950,
    -44.80246, 60.32032
};

static float Georgia[] = {
    37.56048, 45.47804,
    49.25371, 42.67698,
    49.74810, 40.02407,
    38.26687, 42.84842,
    37.56048, 45.47804
};

static float Canary[] = {
    -16.03663, 30.45798,
    -12.24634, 29.59240,
    -13.96021, 27.37160,
    -18.82715, 27.13720,
    -18.34376, 29.65925,
    -16.03663, 30.45798
};

static float Greece[] = {
    28.80912, 36.15079,
    27.09526, 41.22375,
    22.30522, 40.95879,
    22.34916, 35.75949,
    26.21635, 34.39291,
    28.80912, 36.15079
};

static float Finalnd[] = {
    24.81122, 60.93357,
    29.60126, 61.35771,
    30.26044, 65.27780,
    26.08564, 65.00067,
    24.81122, 60.93357
};

static float Great_lakes[] = {
    -92.09488, 46.98230,
    -86.90677, 48.92447,
    -76.22806, 43.64619,
    -82.38040, 41.28006,
    -88.18118, 41.18091,
    -92.09488, 46.98230
};

static float Nuclear_site[] = {
    -115.26013, 35.87970,
    -114.90857, 36.18177,
    -115.04844, 37.60871,
    -117.02598, 38.19386,
    -117.35557, 37.40390,
    -115.26013, 35.87970
};

static float California_mountains[] = {
    -118.02381, 34.11211,
    -118.37537, 37.07739,
    -121.17689, 38.97251,
    -120.82532, 37.64498,
    -118.02381, 34.11211
};

static float Washington_mountains[] = {
    -122.60010, 47.28593,
    -121.11695, 47.27102,
    -120.54566, 43.76236,
    -122.21558, 43.66707,
    -122.60010, 47.28593
};

static float Grand_Canyon[] = {
    -111.61826, 35.22545,
    -111.47544, 37.02670,
    -112.84873, 36.98283,
    -114.93613, 36.04690,
    -113.72764, 35.13566,
    -111.61826, 35.22545
};

static float Denver_mountains[] = {
    -104.40576, 39.75155,
    -104.63648, 38.65334,
    -106.78980, 37.56417,
    -107.88843, 37.53804,
    -106.19654, 40.97368,
    -104.88916, 40.94049,
    -104.40576, 39.75155
};

static float Nile[] = {
    31.32746, 28.89422,
    31.38239, 27.76765,
    32.97566, 26.13181,
    33.05805, 22.91172,
    31.32747, 21.56291,
    31.98315, 23.18417,
    32.12048, 25.65172,
    30.32574, 27.55950,
    30.59352, 28.61441,
    31.32746, 28.89422
};




unsigned locationQuality(float latitude, float longitude)
{
    unsigned quality = 0;

    // --- PRIMARY ZONES (Specific) ---
    if(pointInPolygonF(5, Paris, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, SE_UK, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Ireland_Coast, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(8, Scotland_and_lakes, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Iceland, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(7, Norway, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Alps, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Rome, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Egypt, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Israel, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Cernobyl, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(7, Ukraine, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Dubai, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Mecca, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(12, Himalayas, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, China, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Taiwan, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, NK, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(10, Japan, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Pyrenees, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Galapagos, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(7, Central_America, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Amazon, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Bahamas, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(11, Canada, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(9, Alaska, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(7, Greenland, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Georgia, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Canary, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Greece, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Finalnd, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Great_lakes, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Nuclear_site, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, California_mountains, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(5, Washington_mountains, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(6, Grand_Canyon, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(7, Denver_mountains, latitude, longitude) == 1)
    {
        quality = 1;
    }
    else if(pointInPolygonF(10, Nile, latitude, longitude) == 1)
    {
        quality = 1;
    }
    return quality;
}