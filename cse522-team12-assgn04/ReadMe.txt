CSE 522:Real-Time Embedded Systems 
============================================
Assignment 4 Thread Programming and Device driver in Zephyr RTOS
============================================
Team 12
-----------------------------
Team Members
-----------------------------
Chinmay Bolumbu
ASU ID:1213329869

Rama Kumar Kana Sundara
ASU ID:1213347614
=============================
Files Present
-----------------------------
ReadMe.txt

=============================
Steps to Execute
-----------------------------
1.Connect HCSR04's trigger and echo pins in following order

Device   | Trigger Pin |Echo Pin
-----------------------------------
HCSR0	 | IO8	       |IO12
------------------------------------
HCSR1	 | IO7	       |IO10	
-------------------------------------

2.Copy the following existing files/folders to the folders on right handside of zephyr project directory:

HCSR_app  		   -> ZEPHYR_BASE/samples/
hcsr04   		   ->ZEPHYR_BASE/drivers/sensors
eeprom_24fc256.c 	   ->ZEPHYR_BASE/drivers/flash
eeprom_24fc256.h  hcsr04.h ->ZEPHYR_BASE/include/
Kconfig   		   ->ZEPHYR_BASE/boards/x86/galileo


3.Use ./build.sh in HCSR_app to build and make.Use chmod +x build.sh before using to provide permissions.
4.Shell Commands:

hcsr enable n  (enables n (n=0, 1, or 2, to enable none, HCSR0, or HCSR1)
hcsr start p where p<=512 
hcsr dump p1 p2, where p1<p2
=============================
NOTE:

EEPROM driver is incomplete. 
-----------------------------

