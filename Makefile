CC=g++

all: sim

sim: main.o cache.o memory.o
	$(CC) -o $@ $^
	rm -rf *.o *~

main.o: cache.h

cache.o: cache.h def.h

memory.o: memory.h

prog_sim:
	$(CC) -o $@ -D PROG_SIM cache.cc memory.cc Myloader/Myloader.cc

.PHONY: clean

clean:
	rm -rf sim prog_sim *.o *~ Myloader/*~
