# Makefile for compiling the project
# prerequisite: Linux driver and library for PCA-USB Pro
# pcanbasic: library for PCAN-USB Pro installed in /usr/lib
#
# Project folders
DIR_INC = ./include
#DIR_INC = ./
#DIR_SRC = ./src
DIR_SRC = ./

# build common flags
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -I$(DIR_INC) -lrt #-ggdb # the option "-lrt" is for using the timer related APIs
# -g might not generate debug information for some file, instead use -ggdb
LIBS = -lpcanbasic

#OBJS += canfdTest.o
OBJS += $(DIR_SRC)utility.o
OBJS += $(DIR_SRC)app_canfd.o
OBJS += $(DIR_SRC)app_config.o $(DIR_SRC)app_log.o $(DIR_SRC)app_timer.o
OBJS += $(DIR_SRC)app_file.o
OBJS += $(DIR_SRC)farview_main.o

TARGET = farviewpcan
all := $(TARGET)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< $(LIBS) -o $@

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
	
.PHONY: all clean
