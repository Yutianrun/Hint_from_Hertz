CC:= gcc
CFLAGS:= -O3 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 -Wno-unused-result -Wall -pg
LIBS:= -lpthread -lrt -lm 
UTILS:= ../../util/util.o ../../util/msr-utils.o ../../util/rapl-utils.o ../../util/freq-utils.o

all: bin logs driver_indcpa driver_indcpa_avx


driver_indcpa: bin/obj/driver_indcpa_ref.o $(UTILS)
	$(CC) -o bin/$@ $^ $(LIBS)

driver_indcpa_avx: bin/obj/driver_indcpa_avx.o $(UTILS)
	$(CC) -o bin/$@ $^ $(LIBS)

bin/obj/%.o: %.c | bin
	mkdir -p bin/obj
	$(CC) -c $(CFLAGS) -o $@ $<

bin:
	mkdir -p $@

logs:
	mkdir -p $@

clean:
	rm -rf bin
	rm -rf ../util/*.o

.PHONY: all clean
