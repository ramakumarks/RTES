SDK_HOME = /opt/iot-devkit/1.7.2/sysroots

PATH := $(PATH):$(SDK_HOME)/x86_64-pokysdk-linux/usr/bin/i586-poky-linux

CC = i586-poky-linux-gcc
ARCH = x86
CROSS_COMPILE = i586-poky-linux-
SROOT=$(SDK_HOME)/i586-poky-linux/

APP = a
all: 	
	$(CC) -o $(APP) -pthread -lm -Wall --sysroot=$(SROOT) task.c 

clean:
	rm -f *.o
	rm -f $(APP)
