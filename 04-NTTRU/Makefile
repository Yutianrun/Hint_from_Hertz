# NTTRU frequency side-channel (run-near) – structure aligned with 03-Kyber-CPA-Uncompressed

CC ?= /usr/bin/cc
NTTRU ?= NTTRU/avx2
UTIL := ../util

NISTFLAGS := -Wno-unused-result -mavx2 -mbmi2 -mpopcnt -maes -march=native -mtune=native -O3 -fomit-frame-pointer
NTTRU_CFLAGS := $(NISTFLAGS) -I$(NTTRU)
PATCH_CFLAGS := $(NISTFLAGS) -Ipatch -I$(NTTRU)

BIN := bin
OBJ := $(BIN)/obj

DRIVER_LIBS := -lpthread -lrt -lm
VICTIM_LIBS := -lpthread

# Patch objects (attack-specific glue).
PATCH_OBJS := \
  $(OBJ)/craft_ciphertext.o \
  $(OBJ)/craft_ciphertext_kem.o \
  $(OBJ)/fixed_key.o

# Minimal NTTRU objects needed for keygen/decrypt + crafted ciphertext.
NTTRU_OBJS := \
  $(OBJ)/ntru.o \
  $(OBJ)/poly.o \
  $(OBJ)/consts.o \
  $(OBJ)/ntt.o \
  $(OBJ)/invntt.o \
  $(OBJ)/basemul.o \
  $(OBJ)/baseinv.o \
  $(OBJ)/reduce.o \
  $(OBJ)/add.o \
  $(OBJ)/short.o \
  $(OBJ)/pack.o

# Additional objects needed for KEM
KEM_OBJS := \
  $(OBJ)/kem.o \
  $(OBJ)/aes256ctr.o \
  $(OBJ)/fastrandombytes.o \
  $(OBJ)/kernelrandombytes.o

UTIL_SOURCES := $(UTIL)/util.c $(UTIL)/msr-utils.c $(UTIL)/rapl-utils.c $(UTIL)/freq-utils.c

all: bin data bin/test_nttru_local bin/test_kem_local bin/find_near_z bin/driver_ntru_avx bin/driver_kem_avx

bin/test_nttru_local: $(OBJ)/test_nttru_local.o $(PATCH_OBJS) $(NTTRU_OBJS) | bin
	$(CC) -o $@ $^ $(LDFLAGS) $(VICTIM_LIBS)

bin/test_kem_local: $(OBJ)/test_kem_local.o $(PATCH_OBJS) $(NTTRU_OBJS) $(KEM_OBJS) | bin
	$(CC) -o $@ $^ $(LDFLAGS) $(VICTIM_LIBS) -lcrypto

bin/find_near_z: $(OBJ)/find_near_z.o $(PATCH_OBJS) $(NTTRU_OBJS) | bin
	$(CC) -o $@ $^ $(LDFLAGS)

bin/driver_ntru_avx: driver_ntru_avx.c $(UTIL_SOURCES) | bin
	$(CC) -O0 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 \
	      -Wno-unused-result -Wall -o $@ $^ $(DRIVER_LIBS)

bin/driver_kem_avx: driver_kem_avx.c $(UTIL_SOURCES) | bin
	$(CC) -O0 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 \
	      -Wno-unused-result -Wall -o $@ $^ $(DRIVER_LIBS)

$(OBJ)/%.o: patch/%.c | bin
	@mkdir -p $(dir $@)
	$(CC) $(PATCH_CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(NTTRU)/%.c | bin
	@mkdir -p $(dir $@)
	$(CC) $(NTTRU_CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(NTTRU)/%.s | bin
	@mkdir -p $(dir $@)
	$(CC) $(NTTRU_CFLAGS) -c $< -o $@

bin:
	mkdir -p $(OBJ)

data:
	mkdir -p $@/tmp

clean:
	-rm -rf bin
	-rm -rf data/tmp 2>/dev/null || true

.PHONY: all clean
