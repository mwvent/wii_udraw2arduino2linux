#!/usr/bin/python
#  ii_udraw2arduino2linux
#  The Python Driver
#  By Matthew Watts 2018
#
#  Expects your Arduino is assigned a /dev/ttyUSBx 
#  change below if not the case
#
#  ubuntu packages needed python-evdev xserver-xorg-input-evdev
#  user must be a member of dialout & plugdev although
#  i found on one test system that being a member of plugdev
#  still didnt allow writes to /dev/uinput
#
#  Was very hard to find code examples online for uinput
#  tablet support - hopefully this may be of some use
#  user must be a member of dialout & plugdev
#  
#  A uinput(?)/xorg(?) bug the tablet may be rejected for 
#  no reason - check your Xorg log to see if this is the 
#  case
#  A 'fix' is to add the following to xorg.conf
#  Section "InputClass"
#        Identifier "evdev tablet catchall"
#        MatchIsTablet "on"
#        MatchDevicePath "/dev/input/event*"
#        Driver "evdev"
#  EndSection

import serial
from evdev import UInput, AbsInfo, ecodes as e
import os
import fnmatch

devdir = os.listdir('/dev/')
found =0
for file in devdir:
    if fnmatch.fnmatch(file, 'ttyUSB*'):
        found = 1
        ttyFileName = file

ard = serial.Serial()
ard.port = "/dev/"+ttyFileName
ard.baudrate = 38400
try: 
    ard.open()
except Exception, e:
    print "error open serial port: " + str(e)
    exit()

cap = {
     e.EV_KEY : [e.BTN_TOUCH, e.BTN_TOOL_PEN, e.BTN_STYLUS, e.BTN_STYLUS2],
     e.EV_ABS : [
         (e.ABS_X, AbsInfo(value=1, min=90, max=1650,
                           fuzz=0, flat=0, resolution=1)),
         (e.ABS_Y, AbsInfo(value=2, min=100, max=1400,
                           fuzz=0, flat=0, resolution=1)),
         (e.ABS_PRESSURE, AbsInfo(value=3, min=9, max=240, 
                           fuzz=0, flat=0, resolution=1))
     ],
     e.EV_MSC: [e.MSC_SCAN],
}

minX=90
maxX=1650
minY=100
maxY=1400

# example to ingore 2nd monitor on right
#monOneX=1920
#monTwoX=1024
#newmaxX = int((( float(monOneX) / (monOneX+monTwoX) ) + float(1.15) #) * maxX)
#maxX=newmaxX

capt = {
     e.EV_KEY : [e.BTN_TOUCH, e.BTN_TOOL_PEN, e.BTN_STYLUS, e.BTN_STYLUS2],
     e.EV_ABS : [
         (e.ABS_X, AbsInfo(minX, maxX, 0, 0, 0, 0)),
         (e.ABS_Y, AbsInfo(minY,maxY, 0, 0, 0, 0)),
         (e.ABS_PRESSURE, AbsInfo(8, 180, 0, 10, 0, 0))
     ],
     e.EV_MSC: [e.MSC_SCAN],
}

tablet = UInput(capt, name='udraw', version=0x1)

BTN_PRESSED=0

while 1:
    ard_line = ard.readline()
    ard_event = ard_line.split()
    if len(ard_event) == 6:
        if int(ard_event[0]) < 2000:
            tablet.write(e.EV_ABS, e.ABS_X, int(ard_event[0]))
            tablet.write(e.EV_ABS, e.ABS_Y, 1500-int(ard_event[1]))
            
            if int(ard_event[2]) < 9:
                tablet.write(e.EV_KEY, e.BTN_TOOL_PEN, 1)

        else:
            tablet.write(e.EV_KEY, e.BTN_TOOL_PEN, 0)
        tablet.write(e.EV_ABS, e.ABS_PRESSURE, int(ard_event[2]))
        tablet.write(e.EV_KEY, e.BTN_TOUCH, int(ard_event[3]))
        tablet.write(e.EV_KEY, e.BTN_STYLUS, int(ard_event[4]))
        tablet.write(e.EV_KEY, e.BTN_STYLUS2, int(ard_event[5]))
        tablet.syn()

