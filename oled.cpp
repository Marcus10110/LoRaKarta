#include "oled.h"


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