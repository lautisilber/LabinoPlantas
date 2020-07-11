import time, os, random, threading, math, xlsxwriter, platform
from datetime import datetime
import numpy as np
try:
    import tkinter as tk
    from tkinter import messagebox
except:
    import Tkinter as tk
    from Tkinter import messagebox
from PIL import Image, ImageDraw, ImageTk
import dropbox
from requests.exceptions import ConnectionError, ReadTimeout

### Constantes ###
TOLERANCIA = 15 # Tolerancia en diferencia de porcentaje de humedad entre los valores ideal y real
CANTIDAD_SENSORES = 6
HUMEDAD_IDEAL = [50, 50, 50, 50, 50, 50]
PINES = [2, 3, 4, 5, 6, 7]
REFRESH_RATE = 1 # En segundos
ACCESS_TOKEN = 'LWodg3xsWmAAAAAAAAAAU-X1S5_-yRtUv7_GNFF5ESeUGcrVoHz5XPyfzWUrlaoy'
DIRECTORIO = r'./' # Debe terminar en '/'
UPLOAD_REFRESH_RATE = 15
ONLINE_ENABLED = True

### Variables ###
humedadReal = list()
diferenciaHumedad = list()
aRegar = list()
img = Image.Image
tkImg = Image.Image
xPos = list()
mapper = (0, 0)
startTime, uploadTimeTrack, mins, hours = 0, 0, 0, 0
tkTimeStr = ''
controlIsRunning = False
dbx = dropbox.Dropbox
isOnline, isUploading = False, False
terminate = False
pathToScript = ''

def SensorHumedad():
    val = random.randrange(1, 100)
    return val

def Calibration(val):
    # Toma un valor y devuelve otro con la calibracion aplicada

    newVal = val
    # insertar calculo de calibracion
    return newVal

def init():
    # Chequea que CANTIDAD_SENSORES, HUMEDAD_IDEAL y PINES se correspondan y popula variables
    # humedadReal, diferenciaHumedad y aRegar. Devuelve verdadero si todo OK
    global HUMEDAD_IDEAL, CANTIDAD_SENSORES, PINES, humedadReal
    global diferenciaHumedad, aRegar, img, xPos, mapper, startTime, pathToScript
    if len(HUMEDAD_IDEAL) == CANTIDAD_SENSORES and len(PINES) == CANTIDAD_SENSORES:
        for i in HUMEDAD_IDEAL:
            humedadReal.append(i)
            diferenciaHumedad.append(0)
            aRegar.append(False)
        img, xPos, mapper = CreateBaseImg()
        startTime = time.time()
        if not os.path.exists(r'./Data'):
            os.makedirs(r'./Data')
        if not os.path.exists(r'./Data/Excel'):
            os.makedirs(r'./Data/Excel')
        if not os.path.exists(r'./Data/Excel/Logs'):
            os.makedirs(r'./Data/Excel/Logs')
        if not os.path.exists(r'./Data/Registro'):
            os.makedirs(r'./Data/Registro')
        if 'Linux' in platform.system():
            pathToScript = os.path.dirname(os.path.abspath(__file__))
        elif 'Darwin' in platform.system():
            pathToScript = os.path.dirname(os.path.abspath(__file__))
        initDropbox()

        PrintStatus()
        return True
    else:
        return False

def UpdateHumidity():
    # Refresca los valores de humedad actuales y los guarda en humedadReal
    # Toma la diferencia entre humedad real e ideal y la guarda en diferenciaHumedad
    # Se fija si la diferencia de humedad es mayor a la tolerancia, y si ademas es negativo
    # guarda 'True' en aRegar, si no 'False'

    global humedadReal
    for i in range(len(PINES)):
        # guardar valor del pin en rawVal. Por ahora es un valor al azar
        rawVal = SensorHumedad()
        val = Calibration(rawVal)

        humedadReal[i] = val
        diferenciaHumedad[i] = val - HUMEDAD_IDEAL[i]
        if abs(diferenciaHumedad[i]) > TOLERANCIA and diferenciaHumedad[i] < 0:
            aRegar[i] = True
        else:
            aRegar[i] = False

def UpdateTime():
    global startTime, segs, mins, hours
    global tkTimeStr
    
    m = str(mins)
    if mins < 10:
        m = '0' + str(mins)
    h = str(hours)
    if hours < 10:
        h = '0' + str(hours)
    tkTimeStr = h + ':' + m

    segs = int(time.time() - startTime)
    if math.floor(segs / (60 + (60 * mins) + (60 * 60 * hours))) >= 1:
        mins += 1
        if mins >= 60:
            mins = 0
            hours += 1

def PrintStatus():
    global pathToScript, isOnline

    if len(pathToScript) > 1:
        print('Platform: ' + platform.system())
    print('Online status: ' + str(isOnline))

def Save():
    global humedadReal
    global diferenciaHumedad
    global DIRECTORIO

    # Crear texto con informacion
    string = '[' + datetime.now().strftime("%d/%m/%Y %H:%M:%S") + '] -> (%) '
    for i in humedadReal:
        string += str(i) + ', '
    string += '| (Diferencia) '
    for i in diferenciaHumedad:
        string += str(i) + ', '
    string += '\n\n'


    fileDir = DIRECTORIO + 'Data/Registro/Registro_Humedad_' + datetime.now().strftime("%d-%m-%Y") + '.txt'
    with open(fileDir, 'a+') as file:
        file.write(string)
        file.close()

    return True

def Log():
    # Archivo para que el programa lea mas adelante
    # se escribe 'A/nB/nC/nA/nB/nC/n' donde A es un elemento de humedadReal, B de HUMEDAD_IDEAL y C intervalo de teimpo
    global humedadReal
    global HUMEDAD_IDEAL

    string = datetime.now().strftime("%H:%M") + ' \n'
    for i in humedadReal:
        string += str(i) + ' '
    string += '\n'
    for i in HUMEDAD_IDEAL:
        string += str(i) + ' '
    string += '\n'

    with open(r'./Data/Excel/Logs/' + datetime.now().strftime("%d-%m-%Y") + '.json', 'a+') as file:
        file.write(string)
        file.close()

def CreateExcel(date = datetime.now().strftime("%d-%m-%Y")):
    # what do i do here?

    logReal = list()
    logIdeal = list()
    timeMarks = list()

    with open(r'./Data/Excel/Logs/' + date + '.json', 'r') as file:
        rit = 0
        for line in file:
            if rit == 0:
                timeMarks.append(line.split()[0])
                rit = 1
            elif rit == 1:
                logReal.append(line.split())
                rit = 2
            elif rit == 2:
                logIdeal.append(line.split())
                rit = 0
        file.close()
    
    # write xlsx file
    workbook = xlsxwriter.Workbook(r'./Data/Excel/' + datetime.now().strftime("%d-%m-%Y") + '.xlsx')
    worksheet1 = workbook.add_worksheet()
    worksheet2 = workbook.add_worksheet()

    worksheet1.write(0, 0, 'Info Registrada el dia ' + datetime.now().strftime("%d/%m/%Y"))
    worksheet2.write(1, 0, 'Tiempo')
    worksheet2.write_column(2, 0, timeMarks)
    
    maxPlants = 0
    for i in range(len(timeMarks)):
        worksheet2.write(i + 2, 0, timeMarks[i])
        for n in range(len(logReal[i])):
            worksheet2.write(0, 1 + n * 2, 'Planta ' + str(n + 1))
            worksheet2.write(1, 1 + n * 2, 'Humedad real')
            worksheet2.write(1, 2 + n * 2, 'Humedad ideal')
            worksheet2.write(i + 2, 1 + n * 2, logReal[i][n])
            worksheet2.write(i + 2, 2 + n * 2, logIdeal[i][n])
            if n > maxPlants:
                maxPlants = n
    maxPlants += 1
    
    #chart.add_series(name = title; categories = x axis; values = y axis)
    spaceBetweenCharts = 15
    mainChart = workbook.add_chart({'type' : 'line'})
    charts = list()
    for i in range(maxPlants):
        mainChart.add_series({
            'name' : ['Sheet2', 0, 1 + i * 2],
            'categories' : ['Sheet2', 2, 0, len(timeMarks) + 1, 1],
            'values' : ['Sheet2', 2, 1 + i * 2, len(timeMarks) + 1, 1 + i * 2]
        })

        charts.append(workbook.add_chart({'type' : 'line'}))
        charts[i].add_series({
            'name' : ['Sheet2', 1, 1 + i * 2],
            'categories' : ['Sheet2', 2, 0, len(timeMarks) + 1, 1],
            'values' : ['Sheet2', 2, 1 + i * 2, len(timeMarks) + 1, 1 + i * 2]
        })
        charts[i].add_series({
            'name' : ['Sheet2', 1, 2 + i * 2],
            'categories' : ['Sheet2', 2, 0, len(timeMarks) + 1, 1],
            'values' : ['Sheet2', 2, 2 + i * 2, len(timeMarks) + 1, 2 + i * 2]
        })
        charts[i].set_title({'name' : 'Planta ' + str(i + 1)})
        charts[i].set_x_axis({'name' : 'Tiempo (horas:minutos)'})
        charts[i].set_y_axis({'name' : 'Humedad en %'})
        chartXScale = 10 # bigger number is shorter chart
        worksheet1.insert_chart(1 + spaceBetweenCharts + i * spaceBetweenCharts, 1, charts[i], {'x_scale' : len(timeMarks) / chartXScale, 'y_scale' : 1, 'x_offset': 25, 'y_offset': 10})
    mainChart.set_title({'name' : 'Todas las plantas'})
    mainChart.set_x_axis({'name' : 'Tiempo (horas:minutos)'})
    mainChart.set_y_axis({'name' : 'Humedad en %'})
    worksheet1.insert_chart(1, 1, mainChart, {'x_scale' : len(timeMarks) / chartXScale, 'y_scale' : 1, 'x_offset': 25, 'y_offset': 10})
    workbook.close()

def CreateBaseImg():
    # Creates a base image to graph on top and returns it, the positions 
    # where the columns shall be drawn and a pixel map to map the bar heights
    global CANTIDAD_SENSORES, tkImg
    w, h = 440, 380
    padding = 20

    aImg = Image.new('RGB', (w, h), (255, 255, 255))
    img1 = ImageDraw.Draw(aImg)
    img1.line((padding, padding, padding, h - padding, padding, h - padding, w - padding, h - padding), fill = '#000000', width = 2)

    # x legend
    step = int((w - (padding * 2)) / CANTIDAD_SENSORES)
    aXPos = list()
    for i in range(CANTIDAD_SENSORES):
        text = str(i + 1)
        tw, th = img1.textsize(text)
        aXPos.append(step * i + ((step * (i + 1) - step * i) / 2) + padding  + int(tw / 2))
        img1.text((aXPos[i], h - padding + int(th / 2)), text, font = None, fill = '#000000', align = 'center')
    
    # y legend
    percentages = [0, 25, 50, 75, 100]
    step = int((w - (padding * 2)) / len(percentages) - 1)
    yLegendPos = list()
    for i in range(len(percentages)):
        text = str(percentages[len(percentages) - 1 - i])
        tw, th = img1.textsize(text)
        yLegendPos.append(step * i + padding + int(th / 2))
        img1.text((-int(padding / 5) + int(th / 2), yLegendPos[i]), text, font = None, fill = '#000000', align = 'right')
    
    tkImg = aImg
    aMapper = (padding, h - (padding * 2)) # (padding, height in pixels / 100)
    return aImg, aXPos, aMapper

def Graph(aImg, aXPos, aMapper):
    # takes in a base image, positions of columns (and hence quantity) 
    # and a mapper ((padding, max bar height)) and creates the bars
    # and return the new image
    global HUMEDAD_IDEAL
    global humedadReal
    global tkImg
    width = 10

    newImg = aImg.copy()
    width = int(width/2)
    for i in range(CANTIDAD_SENSORES):
        shape = [(aXPos[i]-width, aMapper[0] + aMapper[1]), (aXPos[i]+width, aMapper[0] + aMapper[1] - int(humedadReal[i] * aMapper[1] / 100))]
        color = '#00ff00'
        if abs(diferenciaHumedad[i]) > TOLERANCIA:
            color = '#ff0000'
        ImageDraw.Draw(newImg).rectangle(shape, fill = color)
        shape = [(aXPos[i]-width-3, aMapper[0] + aMapper[1] - int(HUMEDAD_IDEAL[i] * aMapper[1] / 100)), (aXPos[i]+width+3, aMapper[0] + aMapper[1] - int(HUMEDAD_IDEAL[i] * aMapper[1] / 100))]
        ImageDraw.Draw(newImg).line(shape, fill = '#000000')

    tkImg = newImg 
    return newImg

def initDropbox():
    global ACCESS_TOKEN, dbx, isOnline, ONLINE_ENABLED
    if ONLINE_ENABLED is True:
        dbx = dropbox.Dropbox(ACCESS_TOKEN)
        try:
            dbx.users_get_current_account()
            isOnline = True
        except ConnectionError:
            isOnline = False
    else:
        isOnline = False

def SaveToDropbox(file, destination):
    global dbx, isOnline
    with open(file, 'rb') as f:
        try:
            dbx.files_upload(f.read(), destination, mode=dropbox.files.WriteMode.overwrite)
        except ReadTimeout as error:
            print('Upload timeout error: ' + str(error))
            isOnline = False
        except ConnectionError as error:
            print('Upload connection error: ' + str(error))
            isOnline = False
        f.close()

def DropboxProtocol():
    global dbx, uploadTimeTrack, UPLOAD_REFRESH_RATE, isOnline, ONLINE_ENABLED, isUploading, pathToScript
    if time.time() - uploadTimeTrack >= UPLOAD_REFRESH_RATE and ONLINE_ENABLED is True:
        uploadTimeTrack = time.time()
        try:
            dbx.users_get_current_account()
            isOnline = True
        except ConnectionError:
            isOnline = False
            dbx = dropbox.Dropbox(ACCESS_TOKEN)
            dbx.users_get_current_account()
        
        CreateExcel()
        path = pathToScript + '/Data'   
        files = list()
        for (dirpath, dirnames, filenames) in os.walk(path):
            files.extend([os.path.join(dirpath, file) for file in filenames])
        files = [x for x in files if '.DS_Store' not in x]
        for i in range(len(files)):
            files[i] = files[i][len(pathToScript)::]
        isUploading = True
        for file in files:
            SaveToDropbox(str('.'+file), '/Gustavo' + file)
        isUploading = False
    if ONLINE_ENABLED is False:
        isOnline = False

def gui(master):
    global humedadReal
    global HUMEDAD_IDEAL
    global diferenciaHumedad
    global tkTimeStr, tkImg, img, controlIsRunning, isOnline, ONLINE_ENABLED, REFRESH_RATE, UPLOAD_REFRESH_RATE, TOLERANCIA, terminate

    master.title('Control Humedad GUI')

    frame1 = tk.LabelFrame(master, text='Datos humedad', width=600, height=400)
    hRLabels = list()
    hILabels = list()
    hDLabels = list()
    for i in range(CANTIDAD_SENSORES):
        hILabels.append(tk.Label(frame1, text='  ' + str(HUMEDAD_IDEAL[i]) + '  '))
        hILabels[i].grid(row=0, column=i+1)
    for i in range(CANTIDAD_SENSORES):
        hRLabels.append(tk.Label(frame1, text=str(humedadReal[i])))
        hRLabels[i].grid(row=1, column=i+1)
    for i in range(CANTIDAD_SENSORES):
        hDLabels.append(tk.Label(frame1, text=str(diferenciaHumedad[i])))
        hDLabels[i].grid(row=2, column=i+1)
    tk.Label(frame1, text='Ideal: ').grid(row=0, column=0)
    tk.Label(frame1, text='Real: ').grid(row=1, column=0)
    tk.Label(frame1, text='Diferencia: ').grid(row=2, column=0)
    frame1.grid(row=0, column=0, padx=15, pady=30)

    frame2 = tk.LabelFrame(master)
    graphLabel = tk.Label(frame2, image=ImageTk.PhotoImage(img))
    graphLabel.grid(row=2, column=0)
    frame2.grid(row=0, column=1, rowspan=2, padx=15, pady=15)

    frame4 = tk.Frame(master, bd=1, relief=tk.SUNKEN)
    onlineStatusLabel = tk.Label(frame4, text='Offline', fg = 'red')
    onlineStatusLabel.grid(row=0, column=1)
    def UpdateOnlineStatus():
        global isOnline, isUploading
        if isUploading is True:
            onlineStatusLabel['text'] = 'Uploading...'
            onlineStatusLabel['fg'] = 'blue'
        elif isOnline is True and onlineStatusLabel['text'] != 'Online':
            onlineStatusLabel['text'] = 'Online'
            onlineStatusLabel['fg'] = 'green'
        elif isOnline is False and onlineStatusLabel['text'] != 'Offline':
            onlineStatusLabel['text'] = 'Offline'
            onlineStatusLabel['fg'] = 'red'
    playPausaLabel = tk.Label(frame4, text='En Pausa', fg='red')
    playPausaLabel.grid(row=0, column = 0)
    elapsedLabel = tk.Label(frame4, text='Tiempo transcurrido: 00:00')
    elapsedLabel.grid(row=0, column=2, sticky=tk.E)
    frame4.grid(row=2, column=0, rowspan=3, columnspan=2, sticky=tk.W+tk.E)

    frame3 = tk.Frame(master)
    def AskQuit():
        global terminate
        answer = messagebox.askyesno('Atencion!', 'Seguro querés salir?\nLos datos se guardarán')
        if answer is True:
            terminate = True
            master.destroy()
    tk.Button(frame3, text='Cerrar', command=AskQuit).grid(row=1, column=2, padx=5, pady=3)
    def PlayPause():
        global controlIsRunning
        controlIsRunning = not controlIsRunning
        if controlIsRunning is True:
            playPausaLabel['text'] = 'Corriendo...'
            playPausaLabel['fg'] = 'green'
        elif controlIsRunning is False:
            playPausaLabel['text'] = 'En Pausa'
            playPausaLabel['fg'] = 'red'
    tk.Button(frame3, text='Iniciar/Pausar', command=PlayPause).grid(row=0, column=0, padx=5, pady=3)
    def PopupSetHumidity():
        popup = tk.Toplevel()
        tk.Label(popup, text='Introducir nuevos valores para humedades ideales:').grid(row=0, column=0)
        tFrame1 = tk.LabelFrame(popup)
        entries = list()
        for i in range(CANTIDAD_SENSORES):
            entries.append(tk.Entry(tFrame1, width=3))
            entries[i].grid(row=1, column=i)
        tFrame1.grid(row=1, column=0)
        def OnClick():
            for i in range(CANTIDAD_SENSORES):
                s = entries[i].get()
                if len(s) > 0:
                    if float(s).is_integer() is True:
                        HUMEDAD_IDEAL[i] = int(float(s))
                    else:
                        HUMEDAD_IDEAL[i] = float(s)
                    hILabels[i]['text'] = '  ' + str(HUMEDAD_IDEAL[i]) + '  '
            popup.destroy()
        tF2 = tk.Frame(popup)
        tk.Button(tF2, text='Introducir valores', command=OnClick).grid(row=0, column=0)
        tk.Button(tF2, text='Cancelar', command=popup.destroy).grid(row=0, column=1)
        tF2.grid(row=2, column=0)
        popup.mainloop()
    tk.Button(frame3, text='Elegir humedades ideales', command=PopupSetHumidity).grid(row=0, column=1, padx=5, pady=3)
    def ElegirCrearExcel():
        dates = list()
        for file in os.listdir(r'Data/Excel/Logs/'):
            if file.endswith('.json'):
                dates.append(file[:len(file)-5:])
        popup = tk.Toplevel()
        if len(dates) > 0:
            tk.Label(popup, text='Elegir fecha de datos para crear excel').grid(row=0, column=0)
            var = tk.StringVar(popup)
            try:
                var.set(dates[dates.index(datetime.now().strftime("%d-%m-%Y"))])
            except ValueError:
                var.set(dates[0])
            om = tk.OptionMenu(popup, var, *dates)
            om.grid(row=1, column=0)
            def DoIt(s):
                CreateExcel(s)
                popup.destroy()
            tk.Button(popup, text='Crear excel', command=lambda: DoIt(str(var.get()))).grid(row=1, column=1)
        else:
            tk.Label(popup, text='No hay data para hacer excel').grid(row=0, column=0)
            tk.Button(popup, text='Cerrar', command=popup.destroy).grid(row=1, column=0)
    tk.Button(frame3, text='Crear Excel', command=ElegirCrearExcel).grid(row=0, column=2, padx=5, pady=3)
    def Settings():
        global ONLINE_ENABLED, REFRESH_RATE, UPLOAD_REFRESH_RATE, TOLERANCIA
        popup = tk.Toplevel()
        tk.Label(popup, text='Sensor Refresh').grid(row=0, column=0)
        tk.Label(popup, text='Upload Refresh').grid(row=0, column=1)
        tk.Label(popup, text='Tolerancia').grid(row=0, column=3)
        sRefresh = tk.Entry(popup, width=5)
        sRefresh.insert(0, str(REFRESH_RATE))
        uRefresh = tk.Entry(popup, width=5)
        uRefresh.insert(0, str(UPLOAD_REFRESH_RATE))
        checkVar = tk.BooleanVar(popup)
        checkVar.set(ONLINE_ENABLED)
        onlineCheckBox = tk.Checkbutton(popup, text='Online Activado', variable=checkVar)
        eTolerancia = tk.Entry(popup, width=5)
        eTolerancia.insert(0, str(TOLERANCIA))
        sRefresh.grid(row=1, column=0)
        uRefresh.grid(row=1, column=1)
        onlineCheckBox.grid(row=1, column=2)
        eTolerancia.grid(row=1, column=3)
        settingsFrame = tk.Frame(popup)
        def SettingsOk():
            global ONLINE_ENABLED, REFRESH_RATE, UPLOAD_REFRESH_RATE, TOLERANCIA
            ONLINE_ENABLED = checkVar.get()
            REFRESH_RATE = int(sRefresh.get())
            UPLOAD_REFRESH_RATE = int(uRefresh.get())
            TOLERANCIA = int(eTolerancia.get())
            popup.destroy()
        tk.Button(settingsFrame, text='Aceptar', command=SettingsOk).grid(row=0, column=0)
        tk.Button(settingsFrame, text='Cancelar', command=lambda: popup.destroy()).grid(row=0, column=1)
        settingsFrame.grid(row=2, column=0, columnspan=4)
    tk.Button(frame3, text='Opciones', command=Settings).grid(row=1, column=1, padx=5, pady=3)
    frame3.grid(row=1, column=0, padx=10, pady=10)

    def OnUpdate():
        # To be called from the other thread
        # print('click')
        global isOnline, isUploading
        for i in range(CANTIDAD_SENSORES):
            # So that the gui stays the same size
            #hILabels[i]['text'] = str(HUMEDAD_IDEAL[i])
            hRLabels[i]['text'] = str(humedadReal[i])
            hDLabels[i]['text'] = str(diferenciaHumedad[i])
            elapsedLabel['text'] = 'Tiempo transcurrido: ' + tkTimeStr
            tkGraph = ImageTk.PhotoImage(tkImg)
            graphLabel.configure(image=tkGraph)
            graphLabel.image = tkGraph
            UpdateOnlineStatus()
        root.after(int(REFRESH_RATE * 1000), OnUpdate)
    root.after(int(REFRESH_RATE * 1000), OnUpdate)

def MasterLoop():
    global controlIsRunning, terminate
    while terminate == False:
        if controlIsRunning is True:
            UpdateHumidity()
            UpdateTime()
            Save()
            Log()
            DropboxProtocol()
            Graph(img, xPos, mapper)
            time.sleep(REFRESH_RATE)

### programa ###
if init() is True:
    root = tk.Tk()

    t = threading.Thread(target=MasterLoop, name='thread1')
    t.daemon = False
    t.start()
    gui(root)

    root.mainloop()
