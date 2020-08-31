#include "Arduino.h"
#include "BT_Labino.h"

BT_Labino::BT_Labino(SoftwareSerial *bt, int activityPin, int baud)
{
    _bt = bt;
    _activityPin = activityPin;
    _baud = baud;
}

void BT_Labino::init()
{
    _bt->begin(_baud)
}

void BT_Labino::SetFileName(String name)
{
    _name = name;
}

void BT_Labino::Send(String msg)
{
    
}