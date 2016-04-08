#!/usr/bin/env python
# -*- coding: utf-8 -*-
# (c) ElevenPaths 2016

import sys
import json
import argparse
import datetime

import latch

'''
Python Script to pair a fingerprint ID with a Latch account
'''

## Set default values.
JSON_FILE = "MoneyBox.json"
DEFAULT_OPERATION = "OR"
DEFAULT_MONEY = "112341"

print '-----------------------'
print ' - 11Paths LatchPair -'
print '-----------------------'


## Get all the required arguments to execute the code.
parser = argparse.ArgumentParser()
parser.add_argument('-id', type=int, required=True, help='Index template position on json file (0..255)')
parser.add_argument('-t', '--token', type=str, required=True, help='Latch temporaly pair token')
parser.add_argument('-n', '--name' , help='Optional into to add')

args = parser.parse_args()

## The fingerprint ID must be a valid number, from 0 to 255.
if ((args.id<0) or (args.id>255)):
    print 'ERROR! Valid ID range from 0 to 255'
    exit(1)

if args.name:
    NAME = args.name
else:
    NAME = "Unknown"
    
## Open and Read the json file with the configuration, where must be de Latch appID and secretKey
try:
    print 'Loading config from file: %s' % JSON_FILE
    with open(JSON_FILE) as data_file:
	datas = json.load(data_file)
    
	## Get the appID and secretKey of the app to pair with.
    APP_ID = datas["credentials"]["APP_ID"]
    SECRET_KEY = datas["credentials"]["SECRET_KEY"] 
    
    print 'Latch AppID:  %s' % APP_ID
    print 'Latch Secret: %s' % SECRET_KEY    

	##The fingerprint ID
    ID = args.id    
	
    ##Latch pair code
    pair_code = args.token
    
	## Ask the latch api to pair with your latch app.
    api = latch.Latch(APP_ID,SECRET_KEY)
        
    response = api.pair(pair_code)
       
    responseData = response.get_data()
    
    ## Proccess the latch response, and write the information on the json file
    if responseData:
	if "accountId" in responseData:
	    accountID = responseData["accountId"]
	    print 'Paired accountId: %s' % accountID
	    new_entry = dict()
	    match = False
	    
	    for item in datas["moneybox"]:
		if item["id"] == ID:
		    match = True
		    sys.stdout.write('Warning! ID=%s already exist.\n' % ID)
		    sys.stdout.flush()
			
		    new_entry = item
		    
		    if "latch_accounts" in new_entry:
			for latch_acc in new_entry["latch_accounts"]:
			    if latch_acc["latch_id"] == accountID:
				raise ValueError('Latch accountId exists for ID=' + str(ID))

		    else:
			new_entry["latch_accounts"] = list()
		    
		    new_entry["latch_accounts"].append({"latch_id":accountID,"time":datetime.datetime.now().strftime("%A %d %B %Y @ %H:%M:%S %Z"),"name":NAME})

		    datas["moneybox"].remove(item)

	    
	    if not match:
		# Create new ID
		sys.stdout.write('Writing ID=%s ' % ID)
		sys.stdout.flush()		
		new_entry["id"] = ID
		new_entry["operation"] = DEFAULT_OPERATION
		new_entry["money"] = DEFAULT_MONEY
		new_entry["time"] = datetime.datetime.now().strftime("%A %d %B %Y @ %H:%M:%S %Z")
		new_entry["name"] = NAME
		new_entry["latch_accounts"] = list() 
		new_entry["latch_accounts"].append({"latch_id":accountID,"time":datetime.datetime.now().strftime("%A %d %B %Y @ %H:%M:%S %Z"),"name":NAME})
		
	    datas["moneybox"].append(new_entry)
	    with open(JSON_FILE, 'w') as data_file:
		json.dump(datas, data_file, indent=4,sort_keys=True)		
		    
	    print 'File saved!' 	    
	    
	else:
	    print 'ERROR! Unable to pair by a network error'
    else:
	print 'ERROR! Unable to pair %s' % response.get_error()

except Exception as e:
    print('ABORT! Exception message: ' + str(e))
    exit(1)

print 'END'
exit(0)
