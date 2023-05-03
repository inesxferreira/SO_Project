CC = gcc
CFLAGS = -Wall -g

all: tracer monitor 

tracer: tracer.o

tracer.o: tracer.c

monitor: monitor.o

monitor.o: monitor.c

clean:
	rm -f *.o tracer monitor tmp/*