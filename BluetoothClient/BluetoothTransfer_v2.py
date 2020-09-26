#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Sep  1 20:58:58 2020

@author: lautisilber
"""
import serial, sys, glob, time


class Bluetooth:

    def __init__(self, deviceName = 'ARDUINOBT', baudRate = 38400):
        self.deviceName = deviceName
        self.baudRate = baudRate
        self.btPort = serial.Serial()
        if not self.SetPort():
            print('port error')
        elif not self.BTOK():
            print('no response')
        else:
            print('bluetooth OK')

    def SerialPorts(self):
        if sys.platform.startswith('win'):
            ports = ['COM%s' % (i + 1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            # this excludes your current terminal "/dev/tty"
            ports = glob.glob('/dev/tty[A-Za-z]*')
        elif sys.platform.startswith('darwin'):
            ports = glob.glob('/dev/tty.*')
        else:
            raise EnvironmentError('Unsupported platform')

        result = []
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, serial.SerialException):
                pass

        return result

    def GetPort(self):
        ports = self.SerialPorts()
        for port in ports:
            if self.deviceName in port:
                return port
        return ''

    def SetPort(self):
        port = self.GetPort()
        if port == '':
            print('no valid port!')
            return False
        self.btPort.baudrate = self.baudRate
        self.btPort.timeout = 10
        self.btPort.port = port
        self.btPort.open()
        if self.btPort.is_open:
            print("connected to port '" + self.btPort.name + "'")
            # self.btPort.close()
            return True
        else:
            print("error connecting to port '" + port + "'")
            return False

    def OpenPort(self):
        if not self.btPort.is_open:
            self.btPort.open()

    def SendBT(self, msg):
        assert isinstance(msg, str)
        self.OpenPort()
        self.btPort.write(str.encode(msg))
        print('Sent: ' + msg)
        # self.btPort.close()

    def BTOK(self):
        self.SendBT('OK')
        if self.WaitForResponse('OK'):
            return True
        else:
            return False

    def ReadBT(self):
        self.OpenPort()
        time.sleep(0.01)
        s = ''
        if self.btPort.in_waiting:
            while self.btPort.in_waiting > 0:
                s += str(self.btPort.read().decode())
        return s

    def WaitForResponse(self, code):
        assert isinstance(code, str)
        timeout = 15
        self.OpenPort()
        startTime = time.time()
        while True:
            if self.ReadBT() == code:
                return True
            elif time.time() - startTime >= timeout:
                return False

    def GetFile(self):
        self.SendBT('AD+GETFILE')
        if not self.WaitForResponse('OK'):
            print('get file no response')
            return False
        s = ''
        while (not s.endswith('AD+FILE_END')):
            s += self.ReadBT()
        with open('test.json', 'w') as file:
            file.write(s[len('AD+FILE_BEGIN:') + 1:len('AD+FILE_END')])

def main():
    bt = Bluetooth('ARDUINOBT')

if __name__ == "__main__":
    main()


