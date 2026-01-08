# Kyber INDCPA frequency side-channel attack (uncompressed ciphertext)
# Build against the repo-level Kyber submodule (`../kyber/avx2`) by default,
# and keep attack-specific glue code in `patch/`.

# Keep the same compiler convention as the upstream Makefile.
CC ?= /usr/bin/cc
KYBER ?= ../kyber/avx2
UTIL := ../util

NISTFLAGS := -Wno-unused-result -mavx2 -mbmi2 -mpopcnt -maes -march=native -mtune=native -O3 -fomit-frame-pointer

KYBER_CFLAGS := $(NISTFLAGS) -DKYBER_K=4 -I$(KYBER)
PATCH_CFLAGS := $(NISTFLAGS) -DKYBER_K=4 -Ipatch -I$(KYBER)

BIN := bin
OBJ := $(BIN)/obj

KYBER_LIBS := -lpthread
CHECK_LIBS := -lcrypto -lpthread
DRIVER_LIBS := -lpthread -lrt -lm

# Kyber objects from the external source tree (`$(KYBER)`).
KYBER_OBJS := \
  $(OBJ)/kem.o \
  $(OBJ)/indcpa.o \
  $(OBJ)/polyvec.o \
  $(OBJ)/poly.o \
  $(OBJ)/fq.o \
  $(OBJ)/shuffle.o \
  $(OBJ)/ntt.o \
  $(OBJ)/invntt.o \
  $(OBJ)/basemul.o \
  $(OBJ)/consts.o \
  $(OBJ)/rejsample.o \
  $(OBJ)/cbd.o \
  $(OBJ)/verify.o \
  $(OBJ)/fips202.o \
  $(OBJ)/fips202x4.o \
  $(OBJ)/symmetric-shake.o \
  $(OBJ)/keccak4x/KeccakP-1600-times4-SIMD256.o \
  $(OBJ)/randombytes.o

# Patch objects (attack-specific glue).
PATCH_OBJS := \
  $(OBJ)/craft_ciphertext.o

VICTIM_OBJS := $(OBJ)/test_kyber_indcpa_local.o $(PATCH_OBJS) $(KYBER_OBJS)
CHECK_OBJS := $(OBJ)/check_03_consistency.o $(PATCH_OBJS) $(KYBER_OBJS)

# Util sources
UTIL_SOURCES := $(UTIL)/util.c $(UTIL)/msr-utils.c $(UTIL)/rapl-utils.c $(UTIL)/freq-utils.c

# Driver
DRIVER := driver_indcpa_avx.c

all: bin data bin/test_kyber_indcpa_local bin/driver_indcpa_avx

bin/test_kyber_indcpa_local: $(VICTIM_OBJS) | bin
	$(CC) -o $@ $(VICTIM_OBJS) $(LDFLAGS) $(KYBER_LIBS)

bin/check_03_consistency: $(CHECK_OBJS) | bin
	$(CC) -o $@ $(CHECK_OBJS) $(LDFLAGS) $(CHECK_LIBS)

bin/driver_indcpa_avx: $(DRIVER) $(UTIL_SOURCES) | bin
	$(CC) -O0 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 \
	      -Wno-unused-result -Wall -o $@ $^ $(DRIVER_LIBS)

$(OBJ)/%.o: patch/%.c | bin
	@mkdir -p $(dir $@)
	$(CC) $(PATCH_CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(KYBER)/%.c | bin
	@mkdir -p $(dir $@)
	$(CC) $(KYBER_CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(KYBER)/%.S | bin
	@mkdir -p $(dir $@)
	$(CC) $(KYBER_CFLAGS) -c $< -o $@

bin:
	mkdir -p $(OBJ)

data:
	mkdir -p $@/tmp

clean:
	-rm -rf bin
	-rm -rf data/tmp 2>/dev/null || true

.PHONY: all clean
