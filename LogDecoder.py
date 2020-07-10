#import os
import xlsxwriter

def LogToCSV(logFile):
    assert isinstance(logFile, str)

    numbers = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
    sessionBeginRows = list()
    row = 0

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
                sessionBeginRows.append(row)
            elif split[0] == 'Log':
                csvText += '\n;' + split[1][:len(split[1]) - 1] + ';'
            elif split[0] == 'Hum:' or split[0] == 'Temp:' or split[0] == 'Avg:' or split[0][0] in numbers:
                csvText += split[1] + ';'
            elif split[0] == 'Moisture':
                continue
            elif split[0] == 'FAILED':
                failedCount += 1
            row += 1
            #print(csvText)
            #_ = input()

        csvFile = open('Log.csv', 'w+')
        csvFile.write(csvText)
        csvFile.close()

        return sessionBeginRows

def CSVToExcel(sessionBeginRows):
    assert isinstance(sessionBeginRows, list)

    workbook = xlsxwriter.Workbook('Log.xlsx')
    worksheet1 = workbook.add_worksheet()
    worksheet2 = workbook.add_worksheet()
    csvFile = open('Log.csv', 'r')

    r = 0
    for line in csvFile:
        split = line[:len(line) - 1].split(';')
        for c in range(len(split)):
            if split[c].replace('.', '').isnumeric():
                worksheet1.write(r, c, float(split[c]))
            else:
                worksheet1.write(r, c, split[c])
        r += 1

    mainChart = workbook.add_chart({'type' : 'line'})
    k = 0
    for i in range(len(sessionBeginRows)):
        if i + 1 < len(sessionBeginRows):
            k = sessionBeginRows[i + 1] - sessionBeginRows[i] + 1
        else:
            k = r - sessionBeginRows[i]
        mainChart.add_series({
            'name' : ['Sheet1', 1, 4],
            'categories' : ['Sheet1', k, 1, r + k, 1],
            'values' : ['Sheet1', k, 4, r + k, 4]
        })
    worksheet2.insert_chart('A7', mainChart)

    workbook.close()

sbr = LogToCSV('LOG.TXT')
#CSVToExcel(sbr)
