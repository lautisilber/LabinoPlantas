import os
import tkinter as tk
from tkinter import filedialog
import xlsxwriter

def PathSelect():
    root = tk.Tk()
    root.withdraw()

    logTxtPath = filedialog.askopenfilename(initialdir = os.path.realpath(__file__), title = 'Select log file', filetypes = (('TXT files', '*.TXT'), ('txt files', '*.txt'), ('all files', '*.*')))
    savePath = filedialog.askdirectory(initialdir = os.path.realpath(__file__), title = 'Select save directory')

    print ('Log Source: ' + logTxtPath)
    print ('Save Dir: ' + savePath)
    return logTxtPath, savePath

def LogToCSV(logFile, saveDir):
    assert isinstance(logFile, str)

    numbers = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
    row = 0

    txtFile = open(logFile, 'r')
    if not txtFile:
        print("couldn't open file")
    else:
        csvText = 'Session;Log;Humedad;Temperatura;Hum Suelo Promedio;'
        titleLength = len(csvText)
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
            row += 1

        #formatted [title row, sensor count]
        sessionBeginRows = list()
        tempList = list()
        firstSessionLine = False
        row = 0
        for line in csvText.split('\n'):
            split = line[:len(line) - 1].split(';')
            if firstSessionLine:
                tempList.append(len(split) - 5)
                sessionBeginRows.append(tempList)
                tempList = list()
                firstSessionLine = False
            if line[0] != ';':
                tempList.append(row)
                firstSessionLine = True
            row += 1
        maxSensorCount = max([list(i) for i in zip(*sessionBeginRows)][1])
        csvInsert = ''
        for i in range(maxSensorCount):
            csvInsert += 'Sensor ' + str(i + 1) + ';'
        csvText = csvText[:titleLength] + csvInsert + csvText[titleLength:]

        csvFile = open(os.path.join(saveDir, 'Log.csv'), 'w+')
        csvFile.write(csvText)
        csvFile.close()

    return sessionBeginRows[1:]

def CSVToExcel(saveDir, sessionRows):
    workbook = xlsxwriter.Workbook('Log.xlsx')
    worksheet1 = workbook.add_worksheet()
    worksheet2 = workbook.add_worksheet()
    csvFile = open(os.path.join(saveDir, 'Log.csv'), 'r')

    row = 0
    for line in csvFile:
        split = line[:len(line) - 1].split(';')
        for c in range(len(split)):
            if split[c].replace('.', '').isnumeric():
                worksheet1.write(row, c, float(split[c]))
            else:
                worksheet1.write(row, c, split[c])
        row += 1

    testChart = workbook.add_chart({'type' : 'line'})
    
    testChart.add_series({
            'name' : ['Sheet1', 0, 2],
            'categories' : ['Sheet1', 9, 1, 22, 1],
            'values' : ['Sheet1', 9, 4, 22, 4],
            'marker' : {'type' : 'circle'},
            'line' : {'color' : 'green'},
            'trendline' : {
                'type' : 'polynomial',
                'order' : 3
            }
        })

    #chart.add_series(name = title; categories = x axis; values = y axis)
    sessionCharts = list()
    i = 0
    for r in sessionRows:
        sessionCharts.append(workbook.add_chart({'type' : 'line'}))
        sessionCharts[i].add_series({
            'name' : ['Sheet1', 0, 2],
            'categories' : ['Sheet1', 9, 1, 22, 1],
            'values' : ['Sheet1', 9, 4, 22, 4],
            'marker' : {'type' : 'circle'},
            'line' : {'color' : 'green'},
            'trendline' : {
                'type' : 'polynomial',
                'order' : 3
            }
        })

    worksheet2.insert_chart('A7', testChart)

    workbook.close()

if __name__ == '__main__':
    logPath, saveDir = PathSelect()
    sr = LogToCSV(logPath, saveDir)
    CSVToExcel(saveDir, sr)
