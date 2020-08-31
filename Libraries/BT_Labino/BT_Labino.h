/*
 * BT_Labino.h - library for managing Bluetooth file transfer
 * Created by Lautaro Silbergleit, August 28, 2020
 */
#ifndef BT_Labino_h
#define BT_Labino_h

#include "Arduino.h"
#include <SoftwareSerial.h>

class BT_Labino
{
public:
    BT_Labino(SoftwareSerial *bt, int activityPin = -1, int baud = 38400);
    void init();
    void SetFileName(String name);
    void Send(String msg);

private:
    SoftwareSerial *_bt;
    int _activityPin;
    int _baud;
    String _name = "log.txt";
};

#endif