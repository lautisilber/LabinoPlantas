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
        return None
    
    csvText = 'Session;Log;Humedad;Temperatura;Hum Suelo Promedio;'
    titleLength = len(csvText)
    failedCount = list()
    failedAmount = 0
    for line in txtFile:
        split = line.split()

        if len(split) == 0:
            continue
        if split[0] != 'Log' and split[0] != 'FAILED' and split[0] != 'Moisture' and not split[0].endswith(':'):
            csvText += '\n' + line[:len(line) - 1]
            failedCount.append(failedAmount)
            failedAmount = 0
        elif split[0] == 'Log':
            csvText += '\n;' + split[1][:len(split[1]) - 1] + ';'
        elif split[0] == 'Hum:' or split[0] == 'Temp:' or split[0] == 'Avg:' or split[0][0] in numbers:
            csvText += split[1] + ';'
        elif split[0] == 'Moisture':
            continue
        elif split[0] == 'FAILED':
            failedAmount = int(split[len(split) - 1])
    failedCount.append(failedAmount)

    #formatted [title row, sensor count, session row length, total failed writes in session, session name]
    sessionBeginRows = list()
    #add title row and sensor count
    tempList1 = list()
    firstSessionLine = False
    row = 0
    csvTextSplit = csvText.split('\n')
    for line in csvTextSplit:
        split = line[:len(line) - 1].split(';')
        if firstSessionLine:
            tempList1.append(len(split) - 5)
            sessionBeginRows.append(tempList1)
            tempList1 = list()
            firstSessionLine = False
        if line[0] != ';':
            tempList1.append(row)
            firstSessionLine = True
        row += 1
    #add session row length
    for i in range(len(sessionBeginRows)):
        if i >= len(sessionBeginRows) - 1:
            sessionBeginRows[i].append(row - sessionBeginRows[i][0] - 1)
        else:
            sessionBeginRows[i].append(sessionBeginRows[i + 1][0] - sessionBeginRows[i][0] - 1)
    maxSensorCount = max([list(i) for i in zip(*sessionBeginRows)][1])
    #add total failed writes in session and names
    for i in range(len(sessionBeginRows)):
        sessionBeginRows[i].append(failedCount[i])
        sessionBeginRows[i].append(csvTextSplit[sessionBeginRows[i][0]].replace('\n', ''))
    #insert sensor count in csv file header
    csvInsert = ''
    for i in range(maxSensorCount):
        csvInsert += 'Sensor ' + str(i + 1) + ';'
    csvText = csvText[:titleLength] + csvInsert + csvText[titleLength:]
    csvTextSplit = csvText.split('\n')
    #insert failed writes count
    for i in range(len(sessionBeginRows)):
        if sessionBeginRows[i][3] != 0:
            csvTextSplit[sessionBeginRows[i][0]] = csvTextSplit[sessionBeginRows[i][0]] + ' (Failed writes: ' + str(sessionBeginRows[i][3]) + ')'
    csvText = ''
    for l in csvTextSplit:
        csvText += l + '\n'
        
    #resize sessionBeginRows
    sessionBeginRows = sessionBeginRows[1:]

    csvFile = open(os.path.join(saveDir, 'Log.csv'), 'w+')
    csvFile.write(csvText)
    csvFile.close()

    return sessionBeginRows

def CSVToExcel(saveDir, sessionRows):
    CHART_SPACING = 15

    workbook = xlsxwriter.Workbook('Log.xlsx')
    worksheet1 = workbook.add_worksheet()
    worksheet2 = workbook.add_worksheet()
    csvFile = open(os.path.join(saveDir, 'Log.csv'), 'r')
    csvTextLines = csvFile.readlines()
    csvFile.close()

    row = 0
    totalRows = 0
    for line in csvTextLines:
        totalRows += 1
        split = line[:len(line) - 1].split(';')
        for c in range(len(split)):
            if split[c].replace('.', '').isnumeric():
                worksheet1.write(row, c, float(split[c]))
            else:
                worksheet1.write(row, c, split[c])
        row += 1

    #chart.add_series(name = title; categories = x axis; values = y axis)
    i = 0
    humidityCharts = list()
    temperatureChats = list()
    soilMoistureCharts = list()
    for sess in sessionRows:
        humidityCharts.append(workbook.add_chart({'type' : 'line'}))
        humidityCharts[i].add_series({
            'name' : ['Sheet1', 0, 2],
            'categories' : ['Sheet1', sess[0] + 1, 1, sess[0] + 1 + sess[2], 1],
            'values' : ['Sheet1', sess[0] + 1, 2, sess[0] + 1 + sess[2], 2],
            'marker' : {'type' : 'circle', 'border': {'color': 'green'}, 'fill':   {'color': 'green'}},
            'line' : {'color' : 'green'},
        })
        humidityCharts[i].set_title({'name' : sess[4] + ' - Humedad'})
        temperatureChats.append(workbook.add_chart({'type' : 'line'}))
        temperatureChats[i].add_series({
            'name' : ['Sheet1', 0, 3],
            'categories' : ['Sheet1', sess[0] + 1, 1, sess[0] + 1 + sess[2], 1],
            'values' : ['Sheet1', sess[0] + 1, 3, sess[0] + 1 + sess[2], 3],
            'marker' : {'type' : 'circle', 'border': {'color': 'green'}, 'fill':   {'color': 'green'}},
            'line' : {'color' : 'green'},
        })
        temperatureChats[i].set_title({'name' : sess[4] + ' - Temperatura'})
        soilMoistureCharts.append(workbook.add_chart({'type' : 'line'}))
        soilMoistureCharts[i].add_series({
            'name' : ['Sheet1', 0, 4],
            'categories' : ['Sheet1', sess[0] + 1, 1, sess[0] + 1 + sess[2], 1],
            'values' : ['Sheet1', sess[0] + 1, 4, sess[0] + sess[2], 4],
            'marker' : {'type' : 'circle', 'border': {'color': 'red'}, 'fill':   {'color': 'red'}},
            'line' : {'color' : 'red'},
        })
        soilMoistureCharts[i].add_series({
            'name' : ['Sheet1', 0, 5, 0, 5 + sess[1]],
            'categories' : ['Sheet1', sess[0] + 1, 1, sess[0] + 1 + sess[2], 1],
            'values' : ['Sheet1', sess[0] + 1, 5, sess[0] + sess[2], 5 + sess[1] - 1],
            'marker' : {'type' : 'circle', 'border': {'color': 'blue'}, 'fill':   {'color': 'blue'}},
            'line' : {'color' : 'blue'},
        })
        soilMoistureCharts[i].set_title({'name' : sess[4] + ' - Humedad Suelo'})
        i += 1

    n = 0
    for i in range(len(sessionRows)):
        worksheet2.insert_chart('A' + str(n * CHART_SPACING + 1), humidityCharts[i])
        worksheet2.insert_chart('A' + str((n + 1) * CHART_SPACING + 1), temperatureChats[i])
        worksheet2.insert_chart('A' + str((n + 2) * CHART_SPACING + 1), soilMoistureCharts[i])
        n += 4

    workbook.close()

if __name__ == '__main__':
    logPath, saveDir = PathSelect()
    sr = LogToCSV(logPath, saveDir)
    CSVToExcel(saveDir, sr)
