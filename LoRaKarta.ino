#include "input.h"
#include "radio.h"
#include "lcd.h"
#include "eink.h"
#include "gps.h"

long lastSendTime = 0; // last send time
int interval = 2000;   // interval between sends
byte sendCount = 0;

void OnReceive( const String& message );

void Assert( const String& message );

void setup()
{
    Serial.begin( 9600 ); // initialize serial
    while( !Serial )
        ;
    Serial1.begin( 9600 ); // GPS serial
    Serial.println( "Loading..." );

    if( !Radio::StartRadio( OnReceive ) )
    {
        Assert( "Radio failed to initialize" );
    }
    /*if( !Lcd::StartLcd() )
    {
        Assert( "Lcd failed to initialize" );
    }*/

    /*if( !Eink::StartEInk() )
    {
        Assert( "e-Ink failed to initialize" );
    }*/

    Serial.println( "LoRa init succeeded." );
}

void loop()
{
    while( Serial1.available() )
    {
        GPS::Update();
    }

    if( millis() - lastSendTime > interval )
    {
        String message = "HeLoRa World! "; // send a message
        message += sendCount++;
        Radio::SendMessage( message );
        Serial.println( "Sending " + message );
        lastSendTime = millis();          // timestamp the message
        interval = random( 2000 ) + 1000; // 2-3 seconds
    }
}

void OnReceive( const String& message )
{
    Serial.println( "Message: " + message );
    Serial.println();
}

void Assert( const String& message )
{
    while( true )
        ;
}
