// Radio
#include "radio.h"
#include <LoRa.h>
#include <SPI.h>

// Radio defines:
#define SCK 15
#define MISO 14
#define MOSI 16
#define SS 8
#define RST 4
#define DI0 7
#define BAND 425E6 // 420E6 to 525E6, ham band is 420 to 450MHz
#define PABOOST true

namespace Radio
{
    byte localAddress = 0xBB; // address of this device
    byte destination = 0xFF;  // destination to send to
    byte messageCount = 0;

    ReceiveHandler RxCallback = nullptr;

    void OnReceive( int packet_size );

    bool StartRadio( ReceiveHandler receive_handler )
    {
        RxCallback = receive_handler;
        LoRa.setPins( SS, RST, DI0 ); // set CS, reset, IRQ pin

        if( !LoRa.begin( BAND, PABOOST ) )
        {
            return false;
        }

        LoRa.onReceive( OnReceive );
        LoRa.receive();
        return true;
    }

    void OnReceive( int packet_size )
    {
        if( packet_size == 0 )
            return;

        // read packet header bytes:
        int recipient = LoRa.read();       // recipient address
        byte sender = LoRa.read();         // sender address
        byte incomingMsgId = LoRa.read();  // incoming msg ID
        byte incomingLength = LoRa.read(); // incoming msg length

        String incoming = ""; // payload of packet

        while( LoRa.available() )
        {                                    // can't use readString() in callback, so
            incoming += ( char )LoRa.read(); // add bytes one by one
        }

        if( incomingLength != incoming.length() )
        {           // check length for error
            return; // skip rest of function
        }

        // if the recipient isn't this device or broadcast,
        if( recipient != localAddress && recipient != 0xFF )
        {
            return; // skip rest of function
        }

        if( RxCallback )
            RxCallback( incoming );
    }

    void SendMessage( String message )
    {
        LoRa.beginPacket();             // start packet
        LoRa.write( destination );      // add destination address
        LoRa.write( localAddress );     // add sender address
        LoRa.write( messageCount++ );   // add message ID
        LoRa.write( message.length() ); // add payload length
        LoRa.print( message );          // add payload
        LoRa.endPacket();               // finish packet and send it
        LoRa.receive();
    }
}