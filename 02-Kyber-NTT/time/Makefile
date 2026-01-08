# Makefile for NTT timing tests (AVX2 only)

CC := gcc
CFLAGS_COMMON := -O0 -D_POSIX_SOURCE -D_GNU_SOURCE -Wall -DKYBER_K=4
CFLAGS_AVX2 := $(CFLAGS_COMMON) -m64 -falign-functions=64 -mavx2 -maes -mbmi2 -mpopcnt
LDFLAGS := -pthread

# Directories
KYBER_AVX2_DIR := ../../kyber/avx2
BUILD := build
BUILD_AVX2 := $(BUILD)/avx2

# AVX2 sources
KYBER_AVX2_C_SRCS := poly.c polyvec.c cbd.c consts.c fips202.c fips202x4.c symmetric-shake.c randombytes.c
KYBER_AVX2_ASM_SRCS := fq.S ntt.S invntt.S basemul.S shuffle.S
KECCAK_C_SRCS := keccak4x/KeccakP-1600-times4-SIMD256.c

KYBER_AVX2_OBJS := $(patsubst %.c,$(BUILD_AVX2)/%.o,$(KYBER_AVX2_C_SRCS))
KYBER_AVX2_OBJS += $(patsubst %.S,$(BUILD_AVX2)/%.o,$(KYBER_AVX2_ASM_SRCS))
KYBER_AVX2_OBJS += $(BUILD_AVX2)/KeccakP-1600-times4-SIMD256.o
TEST_AVX2_OBJ := $(BUILD_AVX2)/test_ntt_time.o

.PHONY: all avx2 clean

all: avx2

avx2: test_ntt_time

$(BUILD_AVX2): | $(BUILD)
	mkdir -p $(BUILD_AVX2)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD_AVX2)/%.o: $(KYBER_AVX2_DIR)/%.c | $(BUILD_AVX2)
	$(CC) $(CFLAGS_AVX2) -c -o $@ $<

$(BUILD_AVX2)/%.o: $(KYBER_AVX2_DIR)/%.S | $(BUILD_AVX2)
	$(CC) $(CFLAGS_AVX2) -c -o $@ $< -I$(KYBER_AVX2_DIR)

$(BUILD_AVX2)/KeccakP-1600-times4-SIMD256.o: $(KYBER_AVX2_DIR)/keccak4x/KeccakP-1600-times4-SIMD256.c | $(BUILD_AVX2)
	$(CC) $(CFLAGS_AVX2) -c -o $@ $<

$(BUILD_AVX2)/test_ntt_time.o: test_ntt_time.c | $(BUILD_AVX2)
	$(CC) $(CFLAGS_AVX2) -DKYBER_IMPL_AVX2 -c -o $@ $<

test_ntt_time: $(KYBER_AVX2_OBJS) $(TEST_AVX2_OBJ)
	$(CC) $(CFLAGS_AVX2) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BUILD)
	rm -f test_ntt_time
