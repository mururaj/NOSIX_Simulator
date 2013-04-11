# g++ is the compiler of choice for this simulation
CC=g++

# Specify the compiler options
CFLAGS=-g -c -Wall 

all: hello

hello: generic_queue.o event_queue.o flowGenerator.o sdnSwitch.o sdnController.o NOSIXSimulationVer2.o
	$(CC) generic_queue.o event_queue.o flowGenerator.o sdnSwitch.o sdnController.o NOSIXSimulationVer2.o -o simulator

generic_queue.o: generic_queue.cpp
	$(CC) $(CFLAGS) generic_queue.cpp

event_queue.o: event_queue.cpp
	$(CC) $(CFLAGS) event_queue.cpp

sdnSwitch.o: sdnSwitch.cpp
	$(CC) $(CFLAGS) sdnSwitch.cpp

flowGenerator.o: flowGenerator.cpp
	$(CC) $(CFLAGS) flowGenerator.cpp

sdnController.o: sdnController.cpp
	$(CC) $(CFLAGS) sdnController.cpp

NOSIXSimulationVer2.o: NOSIXSimulationVer2.cpp
	$(CC) $(CFLAGS) NOSIXSimulationVer2.cpp

clean:
	rm -rf *.o simulator
