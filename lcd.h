#pragma once
#include <Arduino.h>
namespace Lcd
{
    bool StartLcd();
    void ShowMessage( String const& message );
    void Clear();
}