#**************************************
#this is make file for build UART DEAMON
#DATE-05_12_2018
#***************************************
CC = gcc
CFLAGS = -I. $(shell pkg-config --cflags libxml-2.0)

LIBS = -lpthread -lmosquitto -lsqlite3 -ljson-c $(shell pkg-config --libs libxml-2.0)
DEPS = common.h Uart.h timestamp.h mqtt.h boot-up-NC.h database.h network-sync.h Uart_for_ARM_final.h
OBJ = Uart_for_ARM_final.o Uart.o timestamp.o mqtt.o boot-up-NC.o database.o network-sync.o

%.o : %.c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS)

uart_deamon : $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	rm -f *.o
clean :
	rm -f uart_deamon *.o
