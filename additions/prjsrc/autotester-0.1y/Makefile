# Makefile for compiling the project
# prerequisite: Linux driver and libraries and headers for PCA-USB Pro
# pcanbasic: library for PCAN-USB Pro installed in /usr/lib
#
# Project source folders
# project header files folder
DIR_INC = ./include
#DIR_INC = ./
# project source files folder
DIR_SRC = ./src
#DIR_SRC = ./

# build common flags
#CC = $(CROSS_COMPILE)gcc
#CFLAGS = -Wall -I$(DIR_INC) -lrt #-ggdb # the option "-lrt" is for using the timer related APIs
# -g might not generate debug information for some file, instead use -ggdb
CFLAGS = -Wall -lrt #-ggdb # the option "-lrt" is for using the timer related APIs

# specify path to standard headers
#CFLAGS += --sysroot=/opt/poky/0.1/sysroots/cortexa76-poky-linux
#LDFLAGS += --sysroot=/opt/poky/0.1/sysroots/cortexa76-poky-linux

CFLAGS += -D__ARM_PCS_VFP

# specify path to specific libraries
#LDFLAGS += -L./lib
# specify specific libraries
#LIBS = -lpcanbasic

#OBJS += canfdTest.o
OBJS += $(DIR_SRC)/utility.o
OBJS += $(DIR_SRC)/app_canfd.o
OBJS += $(DIR_SRC)/app_config.o $(DIR_SRC)/app_log.o $(DIR_SRC)/app_timer.o
OBJS += $(DIR_SRC)/app_file.o
OBJS += $(DIR_SRC)/farview_main.o

TARGET = farview
all := $(TARGET)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) 
#	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^
#	$(CC) $(CFLAGS) -o $@  $^ $(LDFLAGS) $(LIBS)
	$(CC) $(CFLAGS) -o $@  $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJS)
	
.PHONY: all clean
