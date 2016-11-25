CC=g++

all: sim prog_sim

sim: main.o cache.o memory.o
	$(CC) -o $@ $^
	rm -rf *.o *~

main.o: cache.h

cache.o: cache.h def.h

memory.o: memory.h

prog_sim:
	$(CC) -o $@ cache.cc def.h memory.cc Myloader/Myloader.cc

.PHONY: clean

clean:
	rm -rf sim prog_sim *.o *~ Myloader/*~
