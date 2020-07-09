import os

def LogToCSV(logFile):
    assert isinstance(logFile, str)

    numbers = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']

    txtFile = open(logFile, 'r')
    if not txtFile:
        print("couldn't open file")
    else:
        csvText = 'Session;Log;Humedad;Temperatura;Hum Suelo Promedio;Sensores ->'
        failedCount = 0
        for line in txtFile:
            split = line.split()

            if len(split) == 0:
                continue
            if split[0] != 'Log' and split[0] != 'FAILED' and split[0] != 'Moisture' and not split[0].endswith(':'):
                csvText += '\n' + line[:len(line) - 1]
            elif split[0] == 'Log':
                csvText += '\n;' + split[1][:len(split[1]) - 1] + ';'
            elif split[0] == 'Hum:' or split[0] == 'Temp:' or split[0] == 'Avg:' or split[0][0] in numbers:
                csvText += split[1] + ';'
            elif split[0] == 'Moisture':
                continue
            elif split[0] == 'FAILED':
                failedCount += 1

            #print(csvText)
            #_ = input()

        csvFile = open('Log.csv', 'w+')
        csvFile.write(csvText)
        csvFile.close()

LogToCSV('LOG.TXT')
