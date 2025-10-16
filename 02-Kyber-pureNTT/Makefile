CC:= gcc
CFLAGS:= -O0 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 -Wall -DKYBER_K=4
LIBS:= -lpthread -lrt -lm
UTILS:= ../util/util.o ../util/msr-utils.o ../util/rapl-utils.o ../util/freq-utils.o
AVXFLAGS:=-mavx2 -mbmi2 -mpopcnt -maes \
  -march=native -mtune=native -O3 -fomit-frame-pointer
REFOBJ:=obj/poly_ref.o  obj/cbd_ref.o obj/fips202_ref.o obj/randombytes_ref.o obj/symmetric-shake_ref.o obj/ntt_ref.o obj/reduce_ref.o
AVXOBJ:=obj/poly_avx.o obj/consts_avx.o obj/cbd_avx.o obj/fips202_avx.o obj/fips202x4_avx.o obj/symmetric-shake_avx.o 
AVXOBJ_ntt= obj/ntt_avx.o obj/invntt_avx.o  obj/shuffle_avx.o obj/fq_avx.o obj/basemul_avx.o
Driver_obj=obj/driver_avx.o obj/driver_ref.o

all: obj bin out data  driver_ref driver_avx driver_ref_dual driver_avx_dual
# driver_avx

driver_ref: obj/driver_ref.o $(UTILS) $(REFOBJ)
	$(CC) -o bin/$@ $^ $(LIBS)

driver_avx: obj/driver_avx.o $(UTILS) $(AVXOBJ) $(AVXOBJ_ntt)  obj/randombytes_ref.o
	$(CC) $(AVXFLAGS) -o $ bin/$@ $^  $(LIBS) kyber/avx2/keccak4x/KeccakP-1600-times4-SIMD256.c

driver_ref_dual: obj/driver_ref_dual.o $(UTILS) $(REFOBJ)
	$(CC) -o bin/$@ $^ $(LIBS)

driver_avx_dual: obj/driver_avx_dual.o $(UTILS) $(AVXOBJ) $(AVXOBJ_ntt)  obj/randombytes_ref.o
	$(CC) $(AVXFLAGS) -o $ bin/$@ $^  $(LIBS) kyber/avx2/keccak4x/KeccakP-1600-times4-SIMD256.c

obj/driver_ref.o: driver_ref.c
	$(CC) -c $(CFLAGS) -DSLEEP=0 -o  $@ $<

obj/driver_avx.o: driver_avx.c
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -DSLEEP=0 -o $@ $<

obj/driver_ref_dual.o: driver_ref_dual.c
	$(CC) -c $(CFLAGS) -DSLEEP=0 -o  $@ $<

obj/driver_avx_dual.o: driver_avx_dual.c
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -DSLEEP=0 -o $@ $<

$(REFOBJ): obj/%_ref.o: kyber/ref/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(AVXOBJ): obj/%_avx.o: kyber/avx2/%.c
	$(CC) -c  $(CFLAGS) $(AVXFLAGS) -o $@ $< 

$(AVXOBJ_ntt): obj/%_avx.o: kyber/avx2/%.S kyber/avx2/shuffle.inc
	$(CC) -c  $(CFLAGS) $(AVXFLAGS)  -o $@ $<  -I kyber/avx2/

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
