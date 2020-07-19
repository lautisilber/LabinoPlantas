import os, xlsxwriter
from datetime import date
import tkinter as tk
from tkinter import filedialog
import dropbox

class App:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title('Log Reader')
        self.root.minsize(width=100, height=50)
        self.mFrame = tk.Frame(self.root)
        self.mFrame.grid(row=0, column=0)

        self.dbx = dropbox.Dropbox
        self.TOKEN = 'LWodg3xsWmAAAAAAAAAAy9NimXa2nfaUzCALxoKf93Cwo1TCy_Y8Xv6EOcqGwhpz'
        self.isOnline = False

        self.logTxtPath = ''
        self.savePath = ''
        self.sessionInfo = list()

        self.txtText = tk.StringVar()
        self.saveText = tk.StringVar()
        self.statusText = tk.StringVar()
        self.txtText.set('\t\t\t\t\t\t')
        self.saveText.set('\t\t\t\t\t\t')
        self.statusText.set('')

        self.txtLablel = tk.Label(self.mFrame, textvariable=self.txtText)
        self.saveLabel = tk.Label(self.mFrame, textvariable=self.saveText)
        self.statusLabel = tk.Label(self.root, textvariable=self.statusText, relief=tk.SUNKEN, anchor='e')

        self.txtButton = tk.Button(self.mFrame, text='Select log file', fg='black', command=self.FileDialog)
        self.saveButton = tk.Button(self.mFrame, text='Select save directory', fg='black', command=self.DirDialog)
        self.readButton = tk.Button(self.mFrame, text='Read', fg='black', command=self.ReadLog)
        self.dropboxButton = tk.Button(self.mFrame, text='Online', fg='black', state=tk.DISABLED, command=self.initDropbox)
        self.uploadButton = tk.Button(self.mFrame, text='Upload', fg='black', state=tk.DISABLED, command=self.SaveToDropbox)

        self.txtLablel.grid(row=0, column=1)
        self.saveLabel.grid(row=1, column=1)

        self.txtButton.grid(row=0, column=0)
        self.saveButton.grid(row=1, column=0)
        self.readButton.grid(row=2, column=0)
        self.dropboxButton.grid(row=3, column=0)
        self.uploadButton.grid(row=4, column=0)

        self.statusLabel.grid(row=1, column=0, sticky='we')

        self.initDropbox()

        self.root.mainloop()

    def ReadLog(self):
        self.LogToCSV()
        self.CSVToExcel()
        self.CheckIfOnline()
        self.EnableUpload()

    def FileDialog(self):
        self.logTxtPath = filedialog.askopenfilename(initialdir = os.path.realpath(__file__), title = 'Select log file', filetypes = (('TXT files', '*.TXT'), ('txt files', '*.txt'), ('all files', '*.*')))
        self.txtText.set(self.logTxtPath)
        self.CheckIfOnline()
        self.EnableUpload()

    def DirDialog(self):
        self.savePath = filedialog.askdirectory(initialdir = os.path.realpath(__file__), title = 'Select save directory')
        self.saveText.set(self.savePath)
        self.CheckIfOnline()
        self.EnableUpload()

    def initDropbox(self):
        self.dbx = dropbox.Dropbox(self.TOKEN)
        self.CheckIfOnline()
        self.EnableUpload()

    def CheckIfOnline(self):
        try:
            self.dbx.users_get_current_account()
            self.isOnline = True
            self.dropboxButton['state'] = tk.DISABLED
            self.dropboxButton['text'] = 'Online'
        except:
            print('Offline!')
            self.isOnline = False
            self.dropboxButton['state'] = tk.NORMAL
            self.dropboxButton['text'] = 'Go Online'
        self.UpdateStatus()

    def EnableUpload(self):
        if os.path.exists(os.path.join(self.savePath, 'Log.csv')) and os.path.exists(os.path.join(self.savePath, 'Log.xlsx')) and self.isOnline:
            self.uploadButton['state'] = tk.NORMAL
        else:
            self.uploadButton['state'] = tk.DISABLED

    def UpdateStatus(self):
        if self.isOnline:
            self.statusText.set('Online')
        else:
            self.statusText.set('Offline')

    def LogToCSV(self):
        numbers = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
        row = 0

        txtFile = open(self.logTxtPath, 'r')
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

        csvFile = open(os.path.join(self.savePath, 'Log.csv'), 'w+')
        csvFile.write(csvText)
        csvFile.close()

        self.sessionInfo = sessionBeginRows

    def CSVToExcel(self):
        CHART_SPACING = 15
        workbook = xlsxwriter.Workbook(os.path.join(self.savePath, 'Log.xlsx'))
        worksheet1 = workbook.add_worksheet()
        worksheet2 = workbook.add_worksheet()
        csvFile = open(os.path.join(self.savePath, 'Log.csv'), 'r')
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
        for sess in self.sessionInfo:
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
            for n in range(sess[1]):
                soilMoistureCharts[i].add_series({
                'name' : ['Sheet1', 0, 5 + n],
                'categories' : ['Sheet1', sess[0] + 1, 1, sess[0] + 1 + sess[2], 1],
                'values' : ['Sheet1', sess[0] + 1, 5 + n, sess[0] + sess[2], 5 + n],
                'marker' : {'type' : 'circle', 'border': {'color': 'blue'}, 'fill':   {'color': 'blue'}},
                'line' : {'color' : 'blue'},
            })
            soilMoistureCharts[i].set_title({'name' : sess[4] + ' - Humedad Suelo'})
            i += 1

        n = 0
        for i in range(len(self.sessionInfo)):
            worksheet2.insert_chart('A' + str(n * CHART_SPACING + 1), humidityCharts[i])
            worksheet2.insert_chart('A' + str((n + 1) * CHART_SPACING + 1), temperatureChats[i])
            worksheet2.insert_chart('A' + str((n + 2) * CHART_SPACING + 1), soilMoistureCharts[i])
            n += 4

        workbook.close()

    def CheckIfFileExistsDbx(self, dbxPath):
        bExists = False
        try:
            a = self.dbx.files_get_metadata(dbxPath)
            bExists = True
        except:
            bExists = False
        return bExists

    def PromptOverwrite(self):
        t = tk.Toplevel(self.root)
        t.wm_title('File already exists!')
        tk.Label(t, text='File already exists!').grid(row=0, column=1)
        selected = 0
        def PromptChoice(choice):
            selected = choice
            t.quit()
            t.destroy()
        tk.Button(t, text='Cancel', command=lambda: PromptChoice(0)).grid(row=1, column=0)
        tk.Button(t, text='Overwrite', command=lambda: PromptChoice(1)).grid(row=1, column=1)
        tk.Button(t, text='Keep Both', command=lambda: PromptChoice(2)).grid(row=1, column=2)
        print('pinga')
        return selected

    def SaveToDropbox(self):
        
        self.statusText.set('Uploading...')
        self.statusLabel.update_idletasks()

        if self.CheckIfFileExistsDbx(os.path.join(self.savePath, 'Log.csv')):
            selection = self.PromptOverwrite()
            if selection == 0:
                pass
            if selection == 1:
                with open(os.path.join(self.savePath, 'Log.csv'), 'rb') as f:
                    self.dbx.files_upload(f.read(), '/LabinoPlantas/Logs/Log_' + str(date.today()) + '.csv', mode=dropbox.files.WriteMode.overwrite)
        else:
            with open(os.path.join(self.savePath, 'Log.csv'), 'rb') as f:
                self.dbx.files_upload(f.read(), '/LabinoPlantas/Logs/Log_' + str(date.today()) + '.csv')

        if self.CheckIfFileExistsDbx(os.path.join(self.savePath, 'Log.xlsx')):
            selection = self.PromptOverwrite()
            if selection == 0:
                pass
            if selection == 1:
                with open(os.path.join(self.savePath, 'Log.xlsx'), 'rb') as f:
                    self.dbx.files_upload(f.read(), '/LabinoPlantas/Logs/Log_' + str(date.today()) + '.xlsx', mode=dropbox.files.WriteMode.overwrite)
        else:
            with open(os.path.join(self.savePath, 'Log.csv'), 'rb') as f:
                self.dbx.files_upload(f.read(), '/LabinoPlantas/Logs/Log_' + str(date.today()) + '.xlsx')

if __name__ == '__main__':
    app = App()
