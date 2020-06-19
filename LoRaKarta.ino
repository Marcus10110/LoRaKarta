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


TinyGPS gps;

const uint32_t max_gps_age = 10000;


greatcircle_coordinates input_cords;

void setup()
{
    input_cords.latitude1_degrees = 40.785980;
    input_cords.longitude1_degrees = -119.205840;

    last_key_press_ms = millis();
    Serial.begin( 9600 );
    Serial1.begin( 9600 );


    OledSetup();
    KeypadSetup();
    RadioSetup();
}


void loop()
{
    customKeypad.tick();
    char letter = HandleKeypad();
    if( letter != '\0' )
    {
        // handle keypress.
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
        DrawGps( flat, flon, age );
    }
}

void DrawGps( float lat, float lon, uint32_t age )
{
    oled.setCursor( 0, 0 );
    oled.clearToEOL();
    if( age > max_gps_age )
    {
        oled.print( "GPS Lost" );
    }
    else
    {
        input_cords.latitude2_degrees = lat;
        input_cords.longitude2_degrees = lon;
        greatcircle_solution solution = greatcircle_calculate( input_cords );
        auto angle = bearing( input_cords );
        oled.print( solution.distance_miles, 1 );
        oled.print( " miles " );
        oled.print( angle, 1 );
        oled.print( " degrees" );
    }
}

void DrawTextOnLine( const char* message, int line )
{
    oled.setCursor( 0, line );
    oled.clearToEOL();
    oled.print( message );
}

char GetLetter( char key, char count )
{
    if( key == '2' )
        return 'a' + count - 1;
    if( key == '3' )
        return 'd' + count - 1;
    if( key == '4' )
        return 'g' + count - 1;
    if( key == '5' )
        return 'j' + count - 1;
    if( key == '6' )
        return 'm' + count - 1;
    if( key == '7' ) // no Q (PRS)
    {
        if( count == 1 )
            return 'p';
        else
            return 'q' + count - 1;
    }
    if( key == '8' )
        return 't' + count - 1;
    if( key == '9' )
        return 'w' + count - 1; // no Z (WXY)

    return key; // pass 1, *, 0, #.
}

char HandleKeypad()
{
    uint32_t now = millis();
    if( last_key_count != 0 && ( now - last_key_press_ms > 500 ) )
    {
        // commit and move on.
        if( last_key_count > 0 )
        {
            char letter = GetLetter( last_key, last_key_count );
            last_key = '\0';
            last_key_count = 0;
            return letter;
        }
    }

    while( customKeypad.available() )
    {
        keypadEvent e = customKeypad.read();
        if( e.bit.EVENT == KEY_JUST_RELEASED )
        {
            customKeypad.clear(); // ignore anything in the queue (should not have
                                  // anything left)
            if( e.bit.KEY == last_key )
            {
                last_key_count++;
                last_key_press_ms = now;
                if( last_key_count == 3 )
                {
                    // commit, and reset.
                    char letter = GetLetter( last_key, last_key_count );
                    last_key = '\0';
                    last_key_count = 0;
                    last_key_press_ms = now;
                    return letter;
                }
                return '\0';
            }
            else if( last_key_count > 0 )
            {
                // commit last key, start new key.
                char letter = GetLetter( last_key, last_key_count );
                last_key = e.bit.KEY;
                last_key_count = 1;
                last_key_press_ms = now;
                return letter;
            }
            else
            {
                last_key = e.bit.KEY;
                last_key_count = 1;
                last_key_press_ms = now;
                return '\0';
            }
        }
    }
    return '\0';
}
