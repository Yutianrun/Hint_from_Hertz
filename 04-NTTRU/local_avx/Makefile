CC:= gcc
CFLAGS:= -O0 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 -Wno-unused-result -Wall
LIBS:= -lpthread -lrt -lm
UTILS:= ../../util/util.o ../../util/msr-utils.o ../../util/rapl-utils.o ../../util/freq-utils.o
VPATH = ../NTTRU/avx2/
NTTRU = kem.o ntru.o poly.o ntt.o fq.o randombytes.o  aes256ctr.o

all: obj bin out data driver_kem driver_kem_C00 driver_cpa driver_cpa_two_pair

driver_kem: obj/driver_kem.o $(UTILS)
	$(CC) -o bin/$@ $^ $(LIBS)

driver_cpa: obj/driver_cpa.o $(UTILS)
	$(CC) -o bin/$@ $^ $(LIBS)

driver_cpa_two_pair: obj/driver_cpa_two_pair.o $(UTILS)
	$(CC) -o bin/$@ $^ $(LIBS)

driver_kem_C00: obj/driver_kem_C00.o $(UTILS)
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
