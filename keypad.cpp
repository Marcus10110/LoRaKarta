#include <Adafruit_Keypad.h>

#include "keypad.h"


#define R1 10
#define R2 9
#define R3 6
#define R4 5
#define C1 13
#define C2 12
#define C3 11

const byte ROWS = 4;
const byte COLS = 3;

char KEYS[ ROWS ][ COLS ] = { { '1', '2', '3' }, { '4', '5', '6' }, { '7', '8', '9' }, { '*', '0', '#' } };
byte ROW_PINS[ ROWS ] = { R1, R2, R3, R4 };
byte COL_PINS[ COLS ] = { C1, C2, C3 };

uint32_t gLastKeyPressMs = 0;
char gLastKey = '\0';
int gLastKeyCount = 0;

// initialize an instance of class NewKeypad
Adafruit_Keypad gKeypad = Adafruit_Keypad( makeKeymap( KEYS ), ROW_PINS, COL_PINS, ROWS, COLS );


char GetLetter( char key, char count );

void KeypadSetup()
{
    gKeypad.begin();
}

char GetKey()
{
    gKeypad.tick();

    uint32_t now = millis();
    if( gLastKeyCount != 0 && ( now - gLastKeyPressMs > 500 ) )
    {
        // commit and move on.
        if( gLastKeyCount > 0 )
        {
            char letter = GetLetter( gLastKey, gLastKeyCount );
            gLastKey = '\0';
            gLastKeyCount = 0;
            return letter;
        }
    }

    while( gKeypad.available() )
    {
        keypadEvent e = gKeypad.read();
        if( e.bit.EVENT == KEY_JUST_RELEASED )
        {
            gKeypad.clear(); // ignore anything in the queue (should not have
                             // anything left)
            if( e.bit.KEY == gLastKey )
            {
                gLastKeyCount++;
                gLastKeyPressMs = now;
                if( gLastKeyCount == 3 )
                {
                    // commit, and reset.
                    char letter = GetLetter( gLastKey, gLastKeyCount );
                    gLastKey = '\0';
                    gLastKeyCount = 0;
                    gLastKeyPressMs = now;
                    return letter;
                }
                return '\0';
            }
            else if( gLastKeyCount > 0 )
            {
                // commit last key, start new key.
                char letter = GetLetter( gLastKey, gLastKeyCount );
                gLastKey = e.bit.KEY;
                gLastKeyCount = 1;
                gLastKeyPressMs = now;
                return letter;
            }
            else
            {
                gLastKey = e.bit.KEY;
                gLastKeyCount = 1;
                gLastKeyPressMs = now;
                return '\0';
            }
        }
    }
    return '\0';
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
