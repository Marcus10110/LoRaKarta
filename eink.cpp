#include "eink.h"

#define ENABLE_EINK

#ifdef ENABLE_EINK

#include <Adafruit_GFX.h> // Core graphics library
#include "Adafruit_EPD.h"
#include <SD.h>

#endif

#define EPD_CS A5
#define EPD_DC A4
#define SRAM_CS A3
#define EPD_RESET A2 // can set to -1 and share with microcontroller Reset!
#define EPD_BUSY -1  // can set to -1 to not use a pin (will wait a fixed delay)
#define SD_CS A1

#define BUFFPIXEL 5


namespace Eink
{
#ifdef ENABLE_EINK
    uint16_t read16( File& f );
    uint32_t read32( File& f );
    void DrawFile( const char* file_name, int x, int y, bool white_is_clear );

    /* Uncomment the following line if you are using 1.54" tricolor EPD */
    Adafruit_IL0373 display( 152, 152, EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY );
#endif
    bool StartEInk()
    {
#ifdef ENABLE_EINK
        display.begin();
        if( !SD.begin( SD_CS ) )
        {
            return false;
        }
        display.clearBuffer();
#endif
        return true;
    }

    void DrawFile( const char* file_name, int x, int y, bool white_is_clear )
    {
#ifdef ENABLE_EINK
        File bmpFile;
        int bmpWidth, bmpHeight;              // W+H in pixels
        uint8_t bmpDepth;                     // Bit depth (currently must be 24)
        uint32_t bmpImageoffset;              // Start of image data in file
        uint32_t rowSize;                     // Not always = bmpWidth; may have padding
        uint8_t sdbuffer[ 3 * BUFFPIXEL ];    // pixel buffer (R+G+B per pixel)
        uint8_t buffidx = sizeof( sdbuffer ); // Current position in sdbuffer
        boolean goodBmp = false;              // Set to true on valid header parse
        boolean flip = true;                  // BMP is stored bottom-to-top
        int w, h, row, col, x2, y2, bx1, by1;
        uint8_t r, g, b;
        uint32_t pos = 0, startTime = millis();

        if( ( x >= display.width() ) || ( y >= display.height() ) )
            return;

        Serial.println();
        Serial.print( F( "Loading image '" ) );
        Serial.print( file_name );
        Serial.println( '\'' );

        // Open requested file on SD card
        if( ( bmpFile = SD.open( file_name ) ) == ( int )NULL )
        {
            Serial.print( F( "File not found" ) );
            return;
        }

        // Parse BMP header
        if( read16( bmpFile ) == 0x4D42 )
        { // BMP signature
            Serial.print( F( "File size: " ) );
            Serial.println( read32( bmpFile ) );
            ( void )read32( bmpFile );          // Read & ignore creator bytes
            bmpImageoffset = read32( bmpFile ); // Start of image data
            Serial.print( F( "Image Offset: " ) );
            Serial.println( bmpImageoffset, DEC );
            // Read DIB header
            Serial.print( F( "Header size: " ) );
            Serial.println( read32( bmpFile ) );
            bmpWidth = read32( bmpFile );
            bmpHeight = read32( bmpFile );
            if( read16( bmpFile ) == 1 )
            {                                 // # planes -- must be '1'
                bmpDepth = read16( bmpFile ); // bits per pixel
                Serial.print( F( "Bit Depth: " ) );
                Serial.println( bmpDepth );
                if( ( bmpDepth == 24 ) && ( read32( bmpFile ) == 0 ) )
                { // 0 = uncompressed

                    goodBmp = true; // Supported BMP format -- proceed!
                    Serial.print( F( "Image size: " ) );
                    Serial.print( bmpWidth );
                    Serial.print( 'x' );
                    Serial.println( bmpHeight );

                    // BMP rows are padded (if needed) to 4-byte boundary
                    rowSize = ( bmpWidth * 3 + 3 ) & ~3;

                    // If bmpHeight is negative, image is in top-down order.
                    // This is not canon but has been observed in the wild.
                    if( bmpHeight < 0 )
                    {
                        bmpHeight = -bmpHeight;
                        flip = false;
                    }

                    // Crop area to be loaded
                    x2 = x + bmpWidth - 1; // Lower-right corner
                    y2 = y + bmpHeight - 1;
                    if( ( x2 >= 0 ) && ( y2 >= 0 ) )
                    {                 // On screen?
                        w = bmpWidth; // Width/height of section to load/display
                        h = bmpHeight;
                        bx1 = by1 = 0; // UL coordinate in BMP file

                        for( row = 0; row < h; row++ )
                        { // For each scanline...

                            // Seek to start of scan line.  It might seem labor-
                            // intensive to be doing this on every line, but this
                            // method covers a lot of gritty details like cropping
                            // and scanline padding.  Also, the seek only takes
                            // place if the file position actually needs to change
                            // (avoids a lot of cluster math in SD library).
                            if( flip ) // Bitmap is stored bottom-to-top order (normal BMP)
                                pos = bmpImageoffset + ( bmpHeight - 1 - ( row + by1 ) ) * rowSize;
                            else // Bitmap is stored top-to-bottom
                                pos = bmpImageoffset + ( row + by1 ) * rowSize;
                            pos += bx1 * 3; // Factor in starting column (bx1)
                            if( bmpFile.position() != pos )
                            { // Need seek?
                                bmpFile.seek( pos );
                                buffidx = sizeof( sdbuffer ); // Force buffer reload
                            }
                            for( col = 0; col < w; col++ )
                            { // For each pixel...
                                // Time to read more pixel data?
                                if( buffidx >= sizeof( sdbuffer ) )
                                { // Indeed
                                    bmpFile.read( sdbuffer, sizeof( sdbuffer ) );
                                    buffidx = 0; // Set index to beginning
                                }
                                // Convert pixel from BMP to EPD format, push to display
                                b = sdbuffer[ buffidx++ ];
                                g = sdbuffer[ buffidx++ ];
                                r = sdbuffer[ buffidx++ ];

                                uint8_t c = 0;
                                if( ( r < 0x80 ) && ( g < 0x80 ) && ( b < 0x80 ) )
                                {
                                    c = EPD_BLACK; // try to infer black
                                }
                                else if( ( r >= 0x80 ) && ( g >= 0x80 ) && ( b >= 0x80 ) )
                                {
                                    c = EPD_WHITE;
                                }
                                else if( r >= 0x80 )
                                {
                                    c = EPD_RED; // try to infer red color
                                }

                                display.writePixel( col, row, c );
                            } // end pixel
                        }     // end scanline
                    }         // end onscreen
                    // display.display();
                    Serial.print( F( "Loaded in " ) );
                    Serial.print( millis() - startTime );
                    Serial.println( " ms" );
                } // end goodBmp
            }
        }

        bmpFile.close();
        if( !goodBmp )
            Serial.println( F( "BMP format not recognized." ) );
#endif
    }

    // These read 16- and 32-bit types from the SD card file.
    // BMP data is stored little-endian, Arduino is little-endian too.
    // May need to reverse subscript order if porting elsewhere.
#ifdef ENABLE_EINK
    uint16_t read16( File& f )
    {
        uint16_t result;
        ( ( uint8_t* )&result )[ 0 ] = f.read(); // LSB
        ( ( uint8_t* )&result )[ 1 ] = f.read(); // MSB
        return result;
    }

    uint32_t read32( File& f )
    {
        uint32_t result;
        ( ( uint8_t* )&result )[ 0 ] = f.read(); // LSB
        ( ( uint8_t* )&result )[ 1 ] = f.read();
        ( ( uint8_t* )&result )[ 2 ] = f.read();
        ( ( uint8_t* )&result )[ 3 ] = f.read(); // MSB
        return result;
    }
#endif
}