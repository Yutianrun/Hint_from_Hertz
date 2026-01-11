#ifndef FIXED_KEY_H
#define FIXED_KEY_H

#include <stdint.h>

#include "params.h"

// Fill coins with a fixed, deterministic value (used to keep the secret key fixed).
void nttru_fill_coins_fixed(unsigned char coins[N]);

// Alternative deterministic generator (xorshift32) for experiments.
void nttru_fill_coins_xorshift(unsigned char coins[N], uint32_t seed);

#endif

