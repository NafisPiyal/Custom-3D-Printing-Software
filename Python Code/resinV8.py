# If not already installed, install using 'pip install pyserial'
import serial

# Creates a variable called arduino with all the necessary serial information for the specific computer
# CORRECT THE PORT WHEN USING A DIFFERENT MACHINE AND VERIFY THE BAUDRATE
arduino = serial.Serial(port='/dev/tty.usbmodem1101', baudrate=9600, timeout=0.1) 

# Function to send commands to the Arduino
def send_command(command):
    arduino.write(command.encode())

# Event handling for the 'Start Resin' button
def startRes():
    send_command('S')

# Event handling for the 'Stop Resin' button
def stopRes():
    send_command('T')

# Event handling for the 'Prime Resin' button
def primeRes():
    send_command('P')

# Event handling for the 'Reverse Resin' button
def revRes():
    send_command('R')

def setSpeed(speed):
    send_command(f'V{speed}')

def close():
    arduino.close()
    
# Event handling for the 'Set Speed' button
#if event == 'Set Speed':
#    speed = int(values['-UNITVALUE-'])
#    send_command(f'V{speed}')
