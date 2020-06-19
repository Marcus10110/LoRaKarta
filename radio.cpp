
#include "radio.h"

#include <SPI.h>
#include <RH_RF95.h>

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define RF95_FREQ 434.0

RH_RF95 rf95( RFM95_CS, RFM95_INT );

typedef void ( *RxHandler )( uint8_t* buffer, uint8_t length );

RxHandler radio_callback = nullptr;

void RadioSetup( RxHandler callback )
{
    radio_callback = callback;
    pinMode( RFM95_RST, OUTPUT );
    digitalWrite( RFM95_RST, HIGH );

    Serial.println( "Feather LoRa RX Test!" );

    // manual reset
    digitalWrite( RFM95_RST, LOW );
    delay( 10 );
    digitalWrite( RFM95_RST, HIGH );
    delay( 10 );

    while( !rf95.init() )
    {
        Serial.println( "LoRa radio init failed" );
        Serial.println( "Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info" );
        while( 1 )
            ;
    }
    Serial.println( "LoRa radio init OK!" );

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if( !rf95.setFrequency( RF95_FREQ ) )
    {
        Serial.println( "setFrequency failed" );
        while( 1 )
            ;
    }
    Serial.print( "Set Freq to: " );
    Serial.println( RF95_FREQ );

    // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

    // The default transmitter power is 13dBm, using PA_BOOST.
    // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
    // you can set transmitter powers from 5 to 23 dBm:
    rf95.setTxPower( 23, false );
}

void RadioService()
{
    if( rf95.available() )
    {
        // Should be a message for us now
        uint8_t buf[ RH_RF95_MAX_MESSAGE_LEN ];
        uint8_t len = sizeof( buf );
        if( rf95.recv( buf, &len ) )
        {
            if( radio_callback != nullptr )
            {
                radio_callback( buf, len );
            }
        }
    }
}

void RadioTransmit( uint8_t* buffer, uint8_t length )
{
    rf95.send( buffer, length );
    rf95.waitPacketSent();
}