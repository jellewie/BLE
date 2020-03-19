I DO NOT OWN THESE BIN FILES! 
There free to download from manufacturer website. (If you are the owner and want these removed, please just contact me) These files are just included here to make it simpler and be a complete package. In no way I mean harm.

Usefull link
https://forum.arduino.cc/index.php?topic=393655.0

1) Upload 'CCLoader.ino' sketch to Arduino (uno)
2) Wire like this (with 5 to 3.3V scaler resistors)
	Name         	| CC2541	| Arduino
	DEBUG_CLOCK 	| Pin 7		| Pin 5
	DEBUG_DATA    	| Pin 8		| Pin 6
	RESET_N    	| Pin 11	| Pin 4
3) open cmd and go to the location of the file
	D:					(Change to D drive)
	cd D:\Arduino BLE\Firmware updater	(Move to folder)
4) Write and execute this command: (end at 500) (This is V540)
	CCLoader.exe <COM Port> <Firmware.bin> 0
	CCLoader_x86_64.exe 5 CC2541hm10v540.bin 0
	
-YOU NOW NEED A USB<->RS232 COM CABLE/CONNECTOR

5) connect TX and RX to pc, so we can acces the BLE chip by COM
6) send 'AT+SBLUP', continue when 'OK+SBLUP' (make sure pin11 (RESET) is disconnected)
7) open HMSoft.exe and upload 'HMSoft.bin' to the com (This is V705)
