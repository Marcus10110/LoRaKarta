#include "gps.h"

#define gpsPort Serial1
#define GPS_PORT_NAME "Serial1"
#define DEBUG_PORT Serial

namespace GPS
{
    // NMEAGPS Gps;
    gps_fix Fix;

    void StartGps()
    {
        gpsPort.begin( 9600 );
    }

    void Update()
    {
        // while( Gps.available( gpsPort ) )
        {
            // Fix = Gps.read();
        }
    }
    gps_fix& GetGps()
    {
        return Fix;
    }
}