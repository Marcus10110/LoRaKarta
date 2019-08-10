#pragma once

#include <Arduino.h>

namespace Radio
{
    typedef void ( *ReceiveHandler )( const String& );

    bool StartRadio( ReceiveHandler receive_handler );
    void SendMessage( String message );
}