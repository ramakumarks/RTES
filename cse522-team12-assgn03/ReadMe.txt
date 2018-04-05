CSE 522:Real-Time Embedded Systems 
============================================
Assignment 3-Measurement in Zephyr RTOS
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
Report.pdf
measure_12 folder
=============================
Steps to Execute
-----------------------------

1. Use command "chmod +x build.sh to make the bash script executable.
2. Run the command "sudo ./build.sh" to create a folder named build and build using ninja.
3.Copy the image file named 'zephyr.strip' from the path /build/zephyr to the folder named 'kernel' in SD card.
4.To initiate the measurements type following commands in shell:
	-To measure the context switching overhead type the commands inside the quotes: "measure 1".
	-To measure the interrupt latency without background computing type the commands inside the quotes: "measure 2"
	-To measure the interrupt latency with background computing type the commands inside the quotes: "measure 3"
=============================
NOTE:
-Source the project environment file to set the project environtment variables in zephyre project folder:
source zephyr-env.sh
-Make sure your environment is setup by exporting the following environment variables.
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=<path to zephyr-sdk>

