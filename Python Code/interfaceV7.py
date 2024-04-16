# Requires installation of pysimplegui: 'pip install pysimplegui'
import PySimpleGUI as sg

# Requires installation of pyserial: 'pip install pyserial'
import arduinoV7 as ar

# Addresses of button images that are stored locally, if updating the UI to
# be more aesthetic these could be literally anything. Solely used to represent
# the functionality that they perform in the GUI
image_zup = 'button_icons\\white_double_arrow_up.png'
image_zdn = 'button_icons\\white_double_arrow_down.png'
image_yup = 'button_icons\\white_arrow_up.png'
image_ydn = 'button_icons\\white_arrow_down.png'
image_xlf = 'button_icons\\white_arrow_left.png'
image_xrt = 'button_icons\\white_arrow_right.png'
image_tsp = 'button_icons\\transparent.png'
image_clk = 'button_icons\\clockwise.png'
image_cclk = 'button_icons\\counterclockwise.png'

# Sets icon image size for GUI
def getImg(x,y):
    return sg.Image(source=image_tsp,size=(x,y))

units = {'μm.','mm.'}

### MUST BE DEFINED BEFORE PROGRAM IS RUN###
# Steps to MM ratio for specific printer software is being used on
# Originally this was used for all printing but this is only used for
# manual adjustments. These conversion factors can/need to be updated to
# correspond to the specific printer being used whenever generating codes
# for the printer to run on. These values are required in the coordinate gen UI
x_conv = 0.0125
y_conv = 0.0125
z_conv = 0.0176
r_conv = 0.0125

x_step = 1/x_conv
y_step = 1/y_conv
z_step = 1/z_conv
r_step = 1/r_conv

# Converts MM to steps, rounds to nearest step
def manualConversion(move, dir):
    if dir == 'x':
        return round(move * x_step)
    if dir == 'y':
        return round(move * y_step)
    if dir == 'z':
        return round(move * z_step)
    if dir == 'r':
        return round(move * r_step)

# Manual print method for manually controlling stepper motors
def manualPrint(move, dir, unit):
    if unit == 'μm.':
        move = move / 1000
    if dir == "x_pos":
        move = manualConversion(move,'x')
        ar.write(move,0,0,0)
    if dir == "x_neg":
        move = manualConversion(move,'x')
        move = move * -1
        ar.write(move,0,0,0)
    if dir == "y_pos":
        move = manualConversion(move,'y')
        ar.write(0,move,0,0)
    if dir == "y_neg":
        move = manualConversion(move,'y')
        move = move * -1
        ar.write(0,move,0,0)
    if dir == "z_pos":
        move = manualConversion(move,'z')
        ar.write(0,0,move,0)
    if dir == "z_neg":
        move = manualConversion(move,'z')
        move = move * -1
        ar.write(0,0,move,0)
    if dir == "r_pos":
        move = manualConversion(move,'r')
        ar.write(0,0,0,move)
    if dir == "r_neg":
        move = manualConversion(move,'r')
        move = move * -1
        ar.write(0,0,0,move)
    ar.mnlPrt()

# Starts the menu with an initial print status
def printMenu():
    # Icon Size:
    x = 40
    y = 40
    
    menu = [    [getImg(x,y),sg.Text("Manual Controls"),sg.T("  "*15),sg.Text("Resin Controls")],
                [getImg(1.2*x,y),sg.B(image_size=(x,y),image_filename=image_zup,key='z_pos'),getImg(x,y)],

                [getImg(1.2*x,y),sg.B(image_size=(x,y),image_filename=image_yup,key='y_pos'),getImg(x,y),
                 getImg(1.7*x,y),sg.Slider(range=(0,20),orientation='h')],

                [sg.B(image_size=(x,y),image_filename=image_xlf,key='x_neg'),
                 sg.Input(key='-UNITVALUE-',size=(3,5),pad=(0,0)),sg.Combo(list(units),default_value='mm.', size=(4,0), key='-UNITLIST-',pad=(0,0)),
                 sg.B(image_size=(x,y),image_filename=image_xrt,key='x_pos'),
                 getImg(x,y),sg.B('Start Resin'), sg.B('Stop Resin'),sg.B('Prime Resin'), sg.B('Reverse Resin')],

                [sg.B(image_size=(x,y),image_filename=image_clk,key='r_neg'),
                 sg.B(image_size=(x,y),image_filename=image_ydn,key='y_neg',pad=11),
                 sg.B(image_size=(x,y),image_filename=image_cclk,key='r_pos',pad=11)],
                

                [getImg(1.2*x,y),sg.B(image_size=(x,y),image_filename=image_zdn,key='z_neg'),getImg(x,y)],
                
                [getImg(x,y)],

                [sg.B('Set Home'),sg.B('To Home'),sg.B('Get Status'),sg.B('Test')],

                [sg.B('Start/Resume Print'), sg.B('Pause Print'), sg.B('Stop Print'), sg.B('Quit')]
                
                 ]

    # Create the window 
    window = sg.Window('Printer Interface', menu)

    # Core logic of the GUI that is running, 'event' is equivalent to
    # the button names that are defined above. Specific values correspond
    # to 'key(s)' that are set above (such as '-UNITVALUE-'). Events correspond
    # to on screen button clicks. Values are updated whenever a field that they
    # are mapped to is updated. The below 'program,' or core logic of the program,
    # uses error handling to verify input boxes aren't empty but otherwise simply
    # sends directives to the arduino based on selected button 
    while True:
        event, values = window.read()
        if event == sg.WINDOW_CLOSED or event == 'Quit':
            ar.close()
            break

        if event == 'Start/Resume Print':
            ar.startPrt()
        
        if event == 'Pause Print':
            ar.pausePrt()
        
        if event == 'Stop Print':
            ar.stopPrt()
        
        if event == 'Set Home':
            ar.SetHome()

        if event == 'To Home':
            ar.GoHome()
        
        if event == 'Get Status':
            ar.GetPrintStatus()

        if event == 'Test':
            ar.test()
        
        if event == 'Start Resin':
            ar.startRes()

        if event == 'Stop Resin':
            ar.stopRes()

        if event == 'Prime Resin':
            ar.primeRes()

        if event == 'Reverse Resin':
            ar.revRes()   

        if event == 'x_pos':
            try:
                move = int(values['-UNITVALUE-'])
                if move < 0:
                    sg.Popup("Please enter a valid, non-negative integer")
                else:
                    unit = (values['-UNITLIST-'])
                    manualPrint(move,'x_pos',unit)
            except:
                sg.popup("Please specify integer value")

        if event == 'x_neg':
            try:
                move = int(values['-UNITVALUE-'])
                if move < 0:
                    sg.Popup("Please enter a valid, non-negative integer")
                else:
                    unit = (values['-UNITLIST-'])
                    manualPrint(move,'x_neg',unit)
            except:
                sg.popup("Please specify integer value")
                
        if event == 'y_pos':
            try:
                move = int(values['-UNITVALUE-'])
                if move < 0:
                    sg.Popup("Please enter a valid, non-negative integer")
                else:
                    unit = (values['-UNITLIST-'])
                    manualPrint(move,'y_pos',unit)
            except:
                sg.popup("Please specify integer value")
                
        if event == 'y_neg':
            try:
                move = int(values['-UNITVALUE-'])
                if move < 0:
                    sg.Popup("Please enter a valid, non-negative integer")
                else:
                    unit = (values['-UNITLIST-'])
                    manualPrint(move,'y_neg',unit)
            except:
                sg.popup("Please specify integer value")
                
        if event == 'z_pos':
            try:
                move = int(values['-UNITVALUE-'])
                if move < 0:
                    sg.Popup("Please enter a valid, non-negative integer")
                else:
                    unit = (values['-UNITLIST-'])
                    manualPrint(move,'z_pos',unit)
            except:
                sg.popup("Please specify integer value")
                
        if event == 'z_neg':
            try:
                move = int(values['-UNITVALUE-'])
                if move < 0:
                    sg.Popup("Please enter a valid, non-negative integer")
                else:
                    unit = (values['-UNITLIST-'])
                    manualPrint(move,'z_neg',unit)
            except:
                sg.popup("Please specify integer value")

        if event == 'r_pos':
            try:
                move = int(values['-UNITVALUE-'])
                if move < 0:
                    sg.Popup("Please enter a valid, non-negative integer")
                else:
                    unit = (values['-UNITLIST-'])
                    manualPrint(move,'r_pos',unit)
            except:
                sg.popup("Please specify integer value")

        if event == 'r_neg':
            try:
                move = int(values['-UNITVALUE-'])
                if move < 0:
                    sg.Popup("Please enter a valid, non-negative integer")
                else:
                    unit = (values['-UNITLIST-'])
                    manualPrint(move,'r_neg',unit)
            except:
                sg.popup("Please specify integer value")
        
        else:
            continue

# Starts the program
printMenu()