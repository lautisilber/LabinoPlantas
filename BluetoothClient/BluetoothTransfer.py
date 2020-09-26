import serial, sys, glob, time

DEVICE_NAME = 'ARDUINOBT'
BT = serial.Serial()

def SerialPorts():
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

def GetPort(deviceName = DEVICE_NAME):
    ports = SerialPorts()
    for port in ports:
        if deviceName in port:
            return port
    return ''

def SetPort():
    port = GetPort()
    if port == '':
        print('no valid port!')
        return False
    BT.baudrate = 38400
    BT.timeout = 10
    BT.port = port
    BT.open()
    if BT.is_open:
        print("connected to port '" + BT.name + "'")
        # BT.close()
        return True
    else:
        print("error connecting to port '" + port + "'")
        return False

def OpenPort():
    if not BT.is_open:
        BT.open()

def SendBT(msg):
    assert isinstance(msg, str)
    OpenPort()
    BT.write(str.encode(msg))
    # BT.close()

def ReadBT():
    OpenPort()
    time.sleep(0.01)
    s = ''
    if BT.in_waiting:
        while BT.in_waiting > 0:
            s += str(BT.read().decode())
    return s

def WaitForResponse(code):
    assert isinstance(code, str)
    timeout = 15
    OpenPort()
    startTime = time.time()
    while True:
        if ReadBT() == code:
            return True
        elif time.time() - startTime >= timeout:
            return False

def BTOK():
    OpenPort()
    BT.flush()
    SendBT('OK')
    if WaitForResponse('OK'):
        print('bluetooth OK')
        return True
    else:
        print('bluetooth connection error')
        return False

def GetCommand():
    dataIn = ReadBT()
    if dataIn != '':
        if dataIn.startswith('CMD+'):
            SendBT('OK')
            command = dataIn[4:]
            if command.startswith('FILE-'):
                ReceiveFile(command[5:])
        else:
            print(dataIn)

def ReceiveFile(fileName):
    text = ''
    while True:
        text += ReadBT()
        if text.endswith('CMD+END'):
            break
    with open('test.txt', 'w') as file:
        file.write(text[:len(text) - 1 - len('CMD+END')])
        file.close()

def main():
    if not SetPort():
        return
    if not BTOK():
        return
    # SendBT(input('send something: '))
    while True:
        GetCommand()

if __name__ == "__main__":
    main()ja
