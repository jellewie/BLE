# Summary
Arduino Bluetooth module, to (smart?) read data from multiple points.

This can be used to read (or even set) pin states remotely. for example reading the state of doors or windows (read switch), detect if you have post (weight switch). But options are unlimited since the source code is available.

# PRE version (WARNING?)
This is a product in development, Although the newest version should work fine, it has not be fine-tuned to each way you could use it. 
If you decide to order the PCB's, be prepared to so (some) work. Please give feedback if you have found problems I haven’t (yet).
I'm not responsible for mistakes you make, if you use this and you ignite yourself, it's you own responsibility

# How it works
An Arduino Mega (we need a mega since we need multiple Serial pins) will be used as master (see '/Arduino' for firmware)
This Arduino will have its own BLE module, just the normal BLE module but then configured with the AT commands to be the master. 
(This AT codes should be added here when all the test are completed, for now see 'AT Commands BLE module.txt', this is my notes on commands)
<Future room for AT-Commands slave+Master>
The code in the Arduino file should explain itself, it has quite a lot of notes. Also some functions are explained in here.

# Update firmware HM-10
To update the firmware of the HM-10 is quite tricky if you have old modules, check the version with AT+VERSION or AT+VERR?.
See '/Firmware updater' on how to update the HM-10

# Schematic and Gerber (PCB)
The schematic and PCB files are made in EasyEda, they also can be found there, although this is the place with the latest documentation, the Gerber might be more up to date there

-Slave module
https://easyeda.com/jellewietsma/attiny-bluetooth

-Master module
https://easyeda.com/jellewietsma/ble-module-master

# Some more notes:
Useful Guide http://www.martyncurrey.com/hm-10-bluetooth-4ble-modules/
PIO1 low (100ms) when high again !Sleepmode
