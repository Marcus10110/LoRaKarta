#include "oled.h"

#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

namespace
{
    Adafruit_SSD1306 display( 128, 32, &Wire, -1 );
}

void OledSetup()
{
    if( !display.begin( SSD1306_SWITCHCAPVCC, 0x3C ) )
    {
        Serial.println( "SSD1306 allocation failed" );
        for( ;; )
            ;
    }
    display.display();
    display.setTextSize( 1 );              // Normal 1:1 pixel scale
    display.setTextColor( SSD1306_WHITE ); // Draw white text
}
void OledWriteLine( const char* message, int line_number )
{
    display.setCursor( 0, line_number );
    display.write( message );
    display.display();
}
/*
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire oled;
#define I2C_ADDRESS 0x3C

void OledSetup()
{
    oled.begin( &Adafruit128x32, I2C_ADDRESS );
    oled.setFont( Adafruit5x7 );
    oled.displayRemap( true );
    oled.clear();
    oled.setCursor( 0, 2 );
    oled.print( "loading..." );
}

void OledWriteLine( const char* message, int line_number )
{
    oled.setCursor( 0, line_number );
    oled.clearToEOL();
    oled.print( message );
}
*/