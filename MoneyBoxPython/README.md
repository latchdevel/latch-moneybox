# latch-moneybox
Latch - Python integration in a MoneyBox

## Python Libraries

* **latch.py** - Official version of Latch SDK
* **pyfingerprint.py** - Modified version of ZFM-20 API
* **pysearial** - You need to install this library `pip install pyserial`

## Configuration File

* **MoneyBox.json** - General configuration file. It contains the app Latch information, the fingerprint IDs and the latch accounts pair with them. You have to set by hand the APPID and the SECRETKEY, the rest of the file will be automatically write when you run the different scripts. You can also edit it by hand.

## Python Scripts
* **MB_FingerEnroll.py** - Script that allows you to add new fingerprints to the sensor and to the configuration file
* **MB_LatchPair.py** - Script to pair a Latch account. It requires a fingerprint ID, that could previously exit or not in the sensor/configuration file
* **MoneyBox.py** - Main script to make the MoneyBox works.

