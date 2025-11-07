CC:= gcc
CFLAGS:= -O0 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 -Wno-unused-result -Wall -pg
LIBS:= -lpthread -lrt -lm 
UTILS:= ../../../util/util.o ../../../util/msr-utils.o ../../../util/rapl-utils.o ../../../util/freq-utils.o

all: bin data driver_indcpa_avx

driver_indcpa_avx: bin/obj/driver_indcpa_avx.o $(UTILS)
	$(CC) -o bin/$@ $^ $(LIBS)


bin/obj/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

bin:
	mkdir -p $@/obj

data:
	mkdir -p $@/tmp

clean:
	rm -rf bin data/tmp
	rm -rf ../../util/*.o

.PHONY: all clean
