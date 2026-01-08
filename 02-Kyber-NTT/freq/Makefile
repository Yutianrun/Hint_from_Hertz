CC := gcc
CFLAGS := -O0 -D_POSIX_SOURCE -D_GNU_SOURCE -m64 -falign-functions=64 -Wall -DKYBER_K=4
LIBS := -lpthread -lrt -lm
UTILS := ../../util/util.o ../../util/msr-utils.o ../../util/rapl-utils.o ../../util/freq-utils.o
AVXFLAGS := -mavx2 -mbmi2 -mpopcnt -maes -march=native -mtune=native -O3 -fomit-frame-pointer

KYBER_SRC := ../../kyber
BUILD_DIR := bin/build

REF_SRCS := cbd.c fips202.c indcpa.c kem.c ntt.c poly.c polyvec.c randombytes.c reduce.c symmetric-shake.c verify.c
AVX_SRCS := cbd.c consts.c fips202.c fips202x4.c indcpa.c kem.c poly.c polyvec.c randombytes.c rejsample.c symmetric-shake.c verify.c
AVX_ASM_SRCS := basemul.S fq.S invntt.S ntt.S shuffle.S

REF_OBJS := $(addprefix $(BUILD_DIR)/,$(REF_SRCS:.c=_ref.o))
AVX_OBJS := $(addprefix $(BUILD_DIR)/,$(AVX_SRCS:.c=_avx.o))
AVX_ASM_OBJS := $(addprefix $(BUILD_DIR)/,$(AVX_ASM_SRCS:.S=_avx.o))

.PHONY: all clean

all: $(BUILD_DIR) bin data driver_ref driver_avx

driver_ref: $(BUILD_DIR)/driver.ref.o $(UTILS) $(REF_OBJS)
	$(CC) -o bin/$@ $^ $(LIBS)

driver_avx: $(BUILD_DIR)/driver.avx.o $(UTILS) $(AVX_OBJS) $(AVX_ASM_OBJS)
	$(CC) $(AVXFLAGS) -o bin/$@ $^ $(LIBS) $(KYBER_SRC)/avx2/keccak4x/KeccakP-1600-times4-SIMD256.c

$(BUILD_DIR)/driver.ref.o: driver.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -I$(KYBER_SRC) -I$(KYBER_SRC)/ref -o $@ $<

$(BUILD_DIR)/driver.avx.o: driver.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -DUSE_AVX2 -I$(KYBER_SRC) -I$(KYBER_SRC)/avx2 -o $@ $<

$(BUILD_DIR)/%_ref.o: $(KYBER_SRC)/ref/%.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -I$(KYBER_SRC)/ref -o $@ $<

$(BUILD_DIR)/%_avx.o: $(KYBER_SRC)/avx2/%.c | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -I$(KYBER_SRC)/avx2 -o $@ $<

$(BUILD_DIR)/basemul_avx.o: $(KYBER_SRC)/avx2/basemul.S | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -I$(KYBER_SRC)/avx2 -o $@ $<

$(BUILD_DIR)/fq_avx.o: $(KYBER_SRC)/avx2/fq.S | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -I$(KYBER_SRC)/avx2 -o $@ $<

$(BUILD_DIR)/invntt_avx.o: $(KYBER_SRC)/avx2/invntt.S | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -I$(KYBER_SRC)/avx2 -o $@ $<

$(BUILD_DIR)/ntt_avx.o: $(KYBER_SRC)/avx2/ntt.S | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -I$(KYBER_SRC)/avx2 -o $@ $<

$(BUILD_DIR)/shuffle_avx.o: $(KYBER_SRC)/avx2/shuffle.S | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $(AVXFLAGS) -I$(KYBER_SRC)/avx2 -o $@ $<

$(BUILD_DIR):
	mkdir -p $@

bin:
	mkdir -p $@

data:
	mkdir -p $@

clean:
	rm -rf bin/driver_* $(BUILD_DIR)
	rm -rf ../../util/*.o
