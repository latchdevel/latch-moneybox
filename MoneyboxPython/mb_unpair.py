#!/usr/bin/env python
# -*- coding: utf-8 -*-
# (c) ElevenPaths 2016

import latch


'''
Python Script to unpair your latch account, The latch appId and secret ID are necessary
'''

## Set the needed params

APP_ID = "******vajh3********"

SECRET_KEY = "****************UEesj********************"



accountid = "****************************rkYC4m*****************************"


## Ask the latch api to unpair

try:
	api = latch.Latch(APP_ID,SECRET_KEY)



	response = api.unpair(accountid)
except Exception, e:
	print "Latch API error :" + str(e)

## Process the Latch response and confirm the unpair process was right.


if response:

    if response.get_data():

	print response.get_data()

	print "Unpair ok"

    else:

	print "error"

	print response.get_error()

else:

    print "Network error"
