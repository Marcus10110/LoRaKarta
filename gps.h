#pragma once
#include <Arduino.h>
#include <NMEAGPS.h>

#ifdef GPS_FIX_DATE
#error
#endif

namespace GPS
{
    void StartGps();
    void Update();
    gps_fix& GetGps();
}