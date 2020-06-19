#include <SPI.h> // include libraries
#include <Wire.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

// #include <LoRa.h>
#include <TinyGPS.h>
#include "radio.h"

#include "greatcircle.h"

#include "oled.h"
#include "keypad.h"
#include "eink.h"


TinyGPS gps;

const uint32_t max_gps_age = 10000;


greatcircle_coordinates input_cords;

void setup()
{
    input_cords.latitude1_degrees = 40.785980;
    input_cords.longitude1_degrees = -119.205840;
    uint32_t timeout = millis() + 4000;

    Serial.begin( 9600 );
    while( !Serial )
    {
        if( millis() > timeout )
            break;
    }

    Serial.begin( 9600 );
    Serial1.begin( 9600 );

    // OledSetup();
    KeypadSetup();
    // RadioSetup( OnPacket );
    EinkSetup();
    DrawMap();
}


void loop()
{
    char letter = GetKey();
    if( letter != '\0' )
    {
        // handle keypress.
        Serial1.println( letter );
    }


    bool newData = false;
    // int count = 0;
    while( Serial1.available() )
    {
        // if(count++ > 2000)
        // break;
        char c = Serial1.read();
        if( gps.encode( c ) ) // Did a new valid sentence come in?
            newData = true;
    }

    if( newData )
    {
        float flat, flon;
        unsigned long age;
        gps.f_get_position( &flat, &flon, &age );
        // DrawGps( flat, flon, age );
    }
}

void OnPacket( uint8_t* buffer, uint8_t length )
{
}


void DrawGps( float lat, float lon, uint32_t age )
{
    if( age > max_gps_age )
    {
        OledWriteLine( "GPS Lost", 0 );
    }
    else
    {
        input_cords.latitude2_degrees = lat;
        input_cords.longitude2_degrees = lon;
        greatcircle_solution solution = greatcircle_calculate( input_cords );
        auto angle = bearing( input_cords );
        char line[ 24 ];
        snprintf( line, 24, "%i mi, %i deg", ( int )solution.distance_miles, ( int )angle );
        OledWriteLine( line, 0 );
    }
}
