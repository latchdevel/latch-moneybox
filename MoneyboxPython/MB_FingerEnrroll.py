#!/usr/bin/env python
# -*- coding: utf-8 -*-
# (c) ElevenPaths 2016

import os
import re
import sys
import time
import json
import serial
import argparse
import datetime
from   serial.tools  import list_ports
from   pyfingerprint import PyFingerprint

'''

 Python Script to enroll your fingerprint in the fingerprint sensor and save your ID to the json file

'''

## Set default values.
JSON_FILE = "MoneyBox.json"
DEFAULT_OPERATION = "OR"
DEFAULT_MONEY = "112341"

print '-----------------------'
print ' - 11Paths MB Enroll -'
print '-----------------------'

## Get all the required arguments to execute the code.
parser = argparse.ArgumentParser()
parser.add_argument('-f', '--force', action='store_true', help='Force overwrite template id if index position is used')
parser.add_argument('-id', type=int, required=True, help='Index template position on fingerprint sensor database (0..255)')
parser.add_argument('-n', '--name' , help='Optional into to add')

args = parser.parse_args()


## The fingerprint ID must be a valid number, from 0 to 255.
if ((args.id<0) or (args.id>255)):
    print 'ERROR! Valid ID range from 0 to 255'
    exit(1)

############AUXILIARY METHODS#################

# Check the available serial ports and return a list with them.
def get_serial_ports():
    '''
    Returns a list object for all available serial ports
       
    Add on Windows '\\\\.\\COM11'
    '''
    if os.name == 'nt':
        # windows
        _comports = [port for port in list_ports.comports()]
        if _comports.__len__()>0:
            _serial_ports = ['\\\\.\\'+port[0] for port in list_ports.comports()]
	    
        else:
            _serial_ports = []
    else:
        # unix
        _serial_ports = [port[0] for port in list_ports.comports()]
    return _serial_ports

############THE MAIN PROGRAM##############

print 'Searching serial ports...',
serial_ports=get_serial_ports()

if len(serial_ports) == 0:
    print 'No found any serial port'
    exit(1)
else:
    print 'Found %s serial ports' % len(serial_ports)
    for i in range(0,len(serial_ports)):
        print 'Serial #%s name %s' %(i,serial_ports[i])

print ''

# Set fingerprint sensor configuration params
BAUDRATE=57600
TIME_OUT=1

FingerPrint = None

i = 0

# Detect if the fingerprint sensor is connected or not, and get the serial port where it is.
while ((FingerPrint == None) and (i<len(serial_ports))):
    try:
        sys.stdout.write('Checking  %s ' % serial_ports[i])
        ser = serial.Serial(str(serial_ports[i]),baudrate=BAUDRATE,timeout=TIME_OUT)
    except Exception as e:
        ser = None
        print(str(e))  
    if ser:
        print 'OK' 
        if ser.is_open:
            ser.close()   
            if ((FingerPrint == None) and (i<len(serial_ports))):   
                sys.stdout.write('Searching %s for FingerPrint... ' % serial_ports[i])
                sys.stdout.flush()
                
                f = PyFingerprint(str(serial_ports[i]), BAUDRATE, 0xFFFFFFFF, 0x00000000)
                
                if f:
                    try:
                        if (f.verifyPassword() == True):
                            print 'OK!'
                            FingerPrint = str(serial_ports[i])
                        else:
                            print 'password error!'
                        del f
                    except Exception as e:
                        print 'Not found'
                else:
                    print 'ERROR'
    
        else:
            print 'Open %s ERROR' % serial_ports[i]
        ser.close()
    i=i+1
print ''

if ( (FingerPrint == None)):
    print 'ERRROR! FingerPrint is required'
    exit(1)
    
print 'Found FingerPrint connected on %s' % FingerPrint
print '' 

## Once the fingerprint sensor can be used, check if the given ID is in use, to overwrite it or enroll it from the first time
try:
    FP = PyFingerprint(FingerPrint, BAUDRATE, 0xFFFFFFFF, 0x00000000) 
    
    id_in_use=False
    
    ## Gets some sensor information
    print('Currently stored templates: ' + str(FP.getTemplateCount()))        
    for page in range(0,1):
        tableIndex = FP.getTemplateIndex(page)
        for i in range(0, len(tableIndex)):
            if (tableIndex[i]):
                print 'Page %0d Template at position ID=%03d is used' % (page,i)
                if (i==args.id):
                    jsond_in_use=True
    if (id_in_use==True):
        sys.stdout.write('Warning! Index template at position ID=%d is in use. ' % args.id)
        sys.stdout.flush();
        if (args.force==True):
            print 'Overwrite option delete the previous template'
        else:
            raise ValueError('Need --force param to overwrite template ID')
            
    print 'Enrolling at position ID=%d' % args.id
    print 'Waiting for finger...'
    while ( FP.readImage() == False ):
        pass

    ## Converts read image to characteristics and stores it in charbuffer 1
    FP.convertImage(0x01)

    ## Checks if finger is already enrolled
    result = FP.searchTemplate()
    positionNumber = result[0]

    if ( positionNumber >= 0 ):
        raise ValueError('ERROR! Template already exists at position #' + str(positionNumber))

    print('Remove finger...')
    time.sleep(3)
    print('Waiting for same finger again...')

    ## Wait that finger is read again
    while ( FP.readImage() == False ):
        pass

    ## Converts read image to characteristics and stores it in charbuffer 2
    FP.convertImage(0x02)

    ## Compare the finger characteristics of CharBuffer1 with CharBuffer2 and return the accuracy score
    result = FP.compareCharacteristics() 
    
    if (result==0):
        raise ValueError('ERROR! Not match')
    else:
        print 'OK accuracy score %d' % result
	
    ## creates a template
    FP.createTemplate()

    ## Saves template at new position number
    if ( FP.storeTemplate(args.id) == True ):
        print 'Finger enrolled successfully at ID=%d' % args.id

        try:
            print 'Saving ID to config file: %s' % JSON_FILE
	    with open(JSON_FILE) as data_file:
		datas = json.load(data_file)    
		
	    new_entry = dict()
	    match = False
	    for item in datas["moneybox"]:
		if item["id"] == args.id:
		    match = True
	    if (match):
		print 'Warning! ID already exist'
	    else:
		# Create new ID
		new_entry["id"] = args.id
		new_entry["operation"] = DEFAULT_OPERATION
		new_entry["money"] = DEFAULT_MONEY
		new_entry["time"] = datetime.datetime.now().strftime("%A %d %B %Y @ %H:%M:%S %Z")
		if args.name:
		    new_entry["name"] = args.name
		new_entry["latch_accounts"] = list()    
		datas["moneybox"].append(new_entry)
		
		with open(JSON_FILE, 'w') as data_file:
		    json.dump(datas, data_file, indent=4,sort_keys=True)		
		    
		print 'New ID=%s saved!' % args.id
        except:
            print 'ERROR! saving file'
        
    else:
        print 'ERROR! unexpected'


except Exception as e:
    print(str(e))
    del FP
exit(0)
