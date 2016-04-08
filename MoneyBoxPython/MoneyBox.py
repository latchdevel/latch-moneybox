#!/usr/bin/env python
# -*- coding: utf-8 -*-
# (c) ElevenPaths 2016

import os
import re
import sys
import time
import json
import latch
import serial
from serial.tools  import list_ports
from pyfingerprint import PyFingerprint

JSON_FILE = "MoneyBox.json"

print '-----------------------'
print ' - 11Paths MoneyBox  -'
print '-----------------------'

# Method defs
# ------------------------------------------------------------------------- #
#

## Check the availabre serial ports and return a list
def get_serial_ports():
    '''
    Returns a list object for all available serial ports
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


## Search a fingerprint ID in the json file
def MB_searchID(id,file_name=JSON_FILE):
    '''
    Returns a json object with id info or emply if not found id
    '''
    match = False
    result = dict() 	
    try:
	with open(file_name) as data_file:
	    data = json.load(data_file)	    
	result["credentials"] = data["credentials"]
	for item in data["moneybox"]:
	    if item["id"] == id:
		match = True
		result["match"] = item
	if not match:
		print "ID not found in json"
    except:
	print "Exception!"
    return result

## Check the latch status of a latch account and return it
def MB_latchStatus(APP_ID,SECRET_KEY,account_id):
    '''
    Returns latch status of a latch account
    '''
    api = latch.Latch(APP_ID,SECRET_KEY)
    latch_status = False
    try:
	response = api.status(account_id)	
	if response:
	    responseData = response.get_data()
	    if responseData:
		print 'Latch account status:',
		if responseData["operations"][APP_ID]["status"]=="on":
		    print 'on'
		    latch_status = True
		else:
		    print 'off'
	    else:
		print 'ERROR! Unable to get latch status by %s. Set to default "off" (False)' % response.get_error()
	else:
	    print 'ERROR! Unable to get latch status by a network error. Set to default "off" (False)'
    except:
	print 'ERROR! Checking latch status. Set to default "off" (False)'
    return latch_status

## Make MoneyBox to do beep
def MB_beep(self,count=1):
    '''
    Send count beep command to MoneyBox
    '''
    for i in range(0,count):
        self.write(b'5')
        self.flush()
        time.sleep(0.1)
    return

## Make MoneyBox dispend money
def MB_money(self,money):
    '''
    Send money dispend commands to MoneyBox
    '''
    pattern="[1,2,3,4]"
    for match in re.finditer(pattern, money):
	self.write(money[match.start()].encode())
        self.flush()
        time.sleep(300.0 / 1000.0)        
    return

# Checking Serial Ports
# ------------------------------------------------------------------------- #
#

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

# Search serial port for MoneyBox and FingerPrint sensor
# ------------------------------------------------------------------------- #
#
BAUDRATE=57600
TIME_OUT=1

MoneyBox = None
FingerPrint = None
i = 0

while ((MoneyBox == None or FingerPrint == None) and (i<len(serial_ports))):
    try:
        sys.stdout.write('Checking  %s ' % serial_ports[i])
        ser = serial.Serial(str(serial_ports[i]),baudrate=BAUDRATE,timeout=TIME_OUT)
    except Exception as e:
        ser = None
        print(str(e))  
    if ser:
        print 'OK' 
        if ser.is_open:
            
            if (MoneyBox == None):
                sys.stdout.write('Searching %s for MoneyBox... ' % serial_ports[i])
                sys.stdout.flush()
                ser.write(b'9')
                line = ser.readline()
                if line:
                    if (line.find("MoneyBox") !=-1):
                        print 'found! ',line.rstrip()
                        MoneyBox = serial_ports[i]
                        i=i+1
                    else:
                        print 'Not found'
                else:
                    print 'Not found'
                    
                    
            if ((FingerPrint == None) and (i<len(serial_ports))):   
                sys.stdout.write('Searching %s for FingerPrint... ' % serial_ports[i])
                sys.stdout.flush()
                ser.close() 
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

if ((MoneyBox == None) or (FingerPrint == None)):
    print 'ERRROR! MoneyBox and FingerPrint are required'
    exit(1)
    
print 'Found MoneyBox    connected on %s' % MoneyBox
print 'Found FingerPrint connected on %s' % FingerPrint
print '' 

# Main loop
# ------------------------------------------------------------------------- #
#
try:
    MB = serial.Serial(MoneyBox,baudrate=BAUDRATE,timeout=TIME_OUT)
    FP = PyFingerprint(FingerPrint, BAUDRATE, 0xFFFFFFFF, 0x00000000) 
    
    ## Gets some sensor information
    print('Currently stored templates: ' + str(FP.getTemplateCount()))        
    for page in range(0,4):
        tableIndex = FP.getTemplateIndex(page)
        for i in range(0, len(tableIndex)):
            if (tableIndex[i]):
                print 'Page %0d template at position #%03d' % (page,i)
    
    print 'Waiting for finger...'
    while True:
        if (FP.readImage() == True):
            print 'Read!',
            ## Converts read image to characteristics and stores it in charbuffer 1
            FP.convertImage(0x01)      
            
            ## Searchs template
            result = FP.searchTemplate() 
            
            positionNumber = result[0]
            accuracyScore = result[1]            

            if ( positionNumber == -1 ):
                print('No match found!')

                MB_beep(MB,3)
                
            else:
                ID = positionNumber
                sys.stdout.write(' Found template at position ID=%s ' % positionNumber)
                print('the accuracy score is: ' + str(accuracyScore))
		 
		MB_beep(MB,1)
		
		result = MB_searchID(ID)
		
		if "match" in result:
		    print 'Found ID=%s in config file' % ID
		    APP_ID = result["credentials"]["APP_ID"]
		    SECRET_KEY = result["credentials"]["SECRET_KEY"]		    
		    
		    operation = result["match"]["operation"]
		    money = result["match"]["money"]
		    
		    latches = len(result["match"]["latch_accounts"])
		    
		    print 'Latch Account count: %d' % latches
		    print 'String-money to dispend: %s' % money

		    if (latches>1):
			print 'Operation: %s' % operation
			
		    index = 0
		    # Operacion OR
		    if (operation == "OR"):
			dispend = False
			while ((dispend==False) and (index<latches)):
			    print 'Latch account:',
			    print result["match"]["latch_accounts"][index]["latch_id"]
			    if (MB_latchStatus(APP_ID,SECRET_KEY,result["match"]["latch_accounts"][index]["latch_id"])):
			        dispend=True
			    index=index+1		    
		    else:
			dispend = True			
			while ((dispend==True) and (index<latches)):
			    print 'Latch account:',
			    print result["match"]["latch_accounts"][index]["latch_id"]
			    if (not MB_latchStatus(APP_ID,SECRET_KEY,result["match"]["latch_accounts"][index]["latch_id"])):
			        dispend=False			    
			    index=index+1

		    if dispend:
			MB_beep(MB,2)
			sys.stdout.write('Dispending money... ')
			sys.stdout.flush()
			MB_money(MB,money)
			print 'Done!'
		    else:
			print 'No money!!!'
			MB_beep(MB,3)
		    
		else:
		    print 'ERROR! ID=%s not found in config file' % ID
		    MB_beep(MB,5)
            time.sleep(0.5)
            print 'Waiting for finger...'

except Exception as e:
    print('Exception message: ' + str(e))
    MB.close()
    del FP
    
exit(0)
