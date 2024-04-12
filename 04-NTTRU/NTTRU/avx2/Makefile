CC ?= /usr/bin/cc
CFLAGS += -Wall -march=native -mtune=native -O3 -fomit-frame-pointer
#CFLAGS += -DUSE_RDPMC
LDFLAGS += -lcrypto -ldl
SOURCES = kem.c ntru.c poly.c \
  consts.c ntt.s invntt.s basemul.s baseinv.s reduce.s add.s short.s pack.s \
  kernelrandombytes.c fastrandombytes.c aes256ctr.c \
  test/cpucycles.c test/speed.c
HEADERS = params.h kem.h ntru.h poly.h \
  crypto_stream.h randombytes.h kernelrandombytes.h test/cpucycles.h \
  test/speed.h
 
all: test/test_poly test/test_ntru test/test_kem test/test_kem_local test/test_kem_local_C00 test/test_ntru_local test/test_ntru_local_two_pair

test/test_poly: test/test_poly.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $< $(SOURCES) -o $@ $(LDFLAGS)

test/test_ntru: test/test_ntru.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $< $(SOURCES) -o $@ $(LDFLAGS)

test/test_kem: test/test_kem.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $< $(SOURCES) -o $@ $(LDFLAGS)

test/test_kem_local: test/test_kem_local.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $< $(SOURCES) -o $@ $(LDFLAGS) -lpthread

test/test_ntru_local: test/test_ntru_local.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $< $(SOURCES) -o $@ $(LDFLAGS) -lpthread

test/test_ntru_local_two_pair: test/test_ntru_local_two_pair.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $< $(SOURCES) -o $@ $(LDFLAGS) -lpthread

test/test_kem_local_C00: test/test_kem_local_C00.c $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $< $(SOURCES) -o $@ $(LDFLAGS) -lpthread
.PHONY: clean

clean:
	rm -f *~ test/*~
	rm -f test/test_poly
	rm -f test/test_ntru
	rm -f test/test_kem
	rm -f test/test_kem_local
	rm -f test/test_kem_local_C00
