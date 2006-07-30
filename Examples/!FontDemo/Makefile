
TARGET=!runimage

all: $(TARGET)

WARNINGS = -Wall

CC	= gcc
CFLAGS	= -O2 -mthrowback

$(TARGET): main.o 
	$(CC) -o $(TARGET) main.o Desklib:Desklib.o $(CFLAGS)

main.o: main.c 

clean:
	wipe o.* ~C~V 
