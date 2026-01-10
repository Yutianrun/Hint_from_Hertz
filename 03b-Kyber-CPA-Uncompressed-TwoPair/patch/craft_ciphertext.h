#ifndef CRAFT_CIPHERTEXT_H
#define CRAFT_CIPHERTEXT_H

#include <stdint.h>
#include "params.h"
#include "polyvec.h"

#ifndef KYBER_POLYUNCOMPRESSED
#define KYBER_POLYUNCOMPRESSED 512
#endif

#ifndef KYBER_POLYVECUNCOMPRESSED
#define KYBER_POLYVECUNCOMPRESSED (KYBER_K * KYBER_POLYUNCOMPRESSED)
#endif

#ifndef KYBER_UNCOMPRESSED_BYTES
#define KYBER_UNCOMPRESSED_BYTES (KYBER_POLYVECUNCOMPRESSED + KYBER_POLYUNCOMPRESSED)
#endif

void craft_ciphertext(uint8_t c[KYBER_UNCOMPRESSED_BYTES],
                      int16_t z1, int16_t z2, int pair_index);

/* Craft ciphertext with fixed pair0 (1, z_fixed) and target pair (1, z_target). */
void craft_ciphertext_two_pair(uint8_t c[KYBER_UNCOMPRESSED_BYTES],
                               int16_t z_fixed, int16_t z_target,
                               int target_pair_index);

void unpack_ciphertext_uncompressed(polyvec *b, poly *v,
                                    const uint8_t c[KYBER_UNCOMPRESSED_BYTES]);

/* Decrypt uncompressed ciphertext */
void indcpa_dec_uncompressed(uint8_t m[32],
                             const uint8_t c[KYBER_UNCOMPRESSED_BYTES],
                             const uint8_t sk[KYBER_POLYVECBYTES]);

#endif
