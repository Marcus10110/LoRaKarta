#define ASCII_ONLY


#ifdef ASCII_ONLY
#include <U8x8lib.h>
#ifdef U8X8_HAVE_HW_SPI
//#include <SPI.h>
#endif
#else
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif

#include "lcd.h"


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET A0 // Reset pin # (or -1 if sharing Arduino reset pin)

namespace Lcd
{
#ifdef ASCII_ONLY
    // U8X8_PIN_NONE
    U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8( /* reset=*/A0 ); // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED
    bool StartLcd()
    {
        u8x8.begin();
        u8x8.setPowerSave( 0 );
        u8x8.setFont( u8x8_font_chroma48medium8_r );
        return true;
    }

    void ShowMessage( String const& message )
    {
        Clear();
        u8x8.drawString( 0, 0, "Hello World!" );
    }
    void Clear()
    {
        u8x8.clear();
    }
#else
    Adafruit_SSD1306 display( SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET );

    bool StartLcd()
    {
        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if( !display.begin( SSD1306_SWITCHCAPVCC, 0x3C ) )
        { // Address 0x3C for 128x32
            return false;
        }
        display.clearDisplay();
        display.setTextSize( 1 );      // Normal 1:1 pixel scale
        display.setTextColor( WHITE ); // Draw white text
        display.cp437( true );         // Use full 256 char 'Code Page 437' font
        return true;
    }

    void ShowMessage( String const& message )
    {
        Clear();
        display.setCursor( 0, 0 ); // Start at top-left corner
        display.println( message );
    }
    void Clear()
    {
        display.clearDisplay();
    }


#endif
}