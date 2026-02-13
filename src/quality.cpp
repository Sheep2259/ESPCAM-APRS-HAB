#include "geofence.h"
#include <quality.h>

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