#include "input.h"
#include <Key.h>
#include <Keypad.h>

namespace Input
{
    const byte ROWS = 4; // four rows
    const byte COLS = 3; // three columns
    const char keys[ ROWS ][ COLS ] = {
        { '1', '2', '3' }, //
        { '4', '5', '6' }, //
        { '7', '8', '9' }, //
        { '#', '0', '*' }  //
    };
    const byte rowPins[ ROWS ] = { 5, 6, 9, 10 }; // connect to the row pinouts of the keypad
    const byte colPins[ COLS ] = { 11, 12, 13 };  // connect to the column pinouts of the keypad

    Keypad keypad = Keypad( makeKeymap( keys ), rowPins, colPins, ROWS, COLS );


    void Scan()
    {
        char key = keypad.getKey();

        if( key != NO_KEY )
        {
            Serial.println( key );
        }
    }
}