CC:= gcc
CFLAGS:= -O3 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 -Wno-unused-result -Wall -pg
LIBS:= -lpthread -lrt -lm 
UTILS:= ../../util/util.o ../../util/msr-utils.o ../../util/rapl-utils.o ../../util/freq-utils.o

all: obj bin out data  driver_indcpa driver_indcpa_avx


driver_indcpa: obj/driver_indcpa_ref.o $(UTILS) 
	$(CC) -o bin/$@ $^ $(LIBS)

driver_indcpa_avx: obj/driver_indcpa_avx.o $(UTILS) 
	$(CC) -o bin/$@ $^ $(LIBS)

obj/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

obj:
	mkdir -p $@

bin:
	mkdir -p $@

out:
	mkdir -p $@

data:
	mkdir -p $@
	
clean:
	rm -rf bin obj out
	rm -rf ../util/*.o

.PHONY: all clean
