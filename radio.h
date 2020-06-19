#include <Arduino.h>

typedef void ( *RxHandler )( uint8_t* buffer, uint8_t length );

void RadioSetup( RxHandler callback );

void RadioService();

void RadioTransmit( uint8_t* buffer, uint8_t length );