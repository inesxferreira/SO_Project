CC = gcc
CFLAGS = -Wall -g

all: tmp log tracer monitor 

tmp: 
	mkdir -p tmp

log: 
	mkdir -p log

tracer: tracer.o

tracer.o: tracer.c

monitor: monitor.o

monitor.o: monitor.c

clean:
	rm -f *.o tracer monitor
	rm -rf tmp log