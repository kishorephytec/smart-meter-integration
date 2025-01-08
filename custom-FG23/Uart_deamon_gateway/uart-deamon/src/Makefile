#**************************************
#this is make file for build UART DEAMON
#DATE-05_12_2018
#***************************************
CC = gcc
CFLAGS = -I. $(shell pkg-config --cflags libxml-2.0) $(shell pkg-config --cflags dbus-1)

LIBS = -lpthread -lmosquitto -lcjson $(shell pkg-config --libs libxml-2.0) $(shell pkg-config --libs dbus-1)
DEPS = common.h Hif.h PIPE.h Uart.h UDP_Server.h timestamp.h mqtt.h system_ip.h dlms_decode.h
OBJ = Uart_for_ARM_final.o Hif.o PIPE.o Uart.o UDP_Server.o timestamp.o mqtt.o system_ip.o dlms_decode.o

%.o : %.c $(DEPS) 
	$(CC) -c -o $@ $< $(CFLAGS)

uart_deamon : $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	rm -f *.o
clean :
	rm -f uart_deamon *.o
