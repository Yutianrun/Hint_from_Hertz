#ifndef INDCPA_H
#define INDCPA_H

#include <stdint.h>
#include "params.h"
#include "polyvec.h"

#define gen_matrix KYBER_NAMESPACE(gen_matrix)
void gen_matrix(polyvec *a, const uint8_t seed[KYBER_SYMBYTES], int transposed);
#define indcpa_keypair KYBER_NAMESPACE(indcpa_keypair)
void indcpa_keypair(uint8_t pk[KYBER_INDCPA_PUBLICKEYBYTES],
                    uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES]);

#define indcpa_enc KYBER_NAMESPACE(indcpa_enc)
void indcpa_enc(uint8_t c[KYBER_INDCPA_BYTES],
                const uint8_t m[KYBER_INDCPA_MSGBYTES],
                const uint8_t pk[KYBER_INDCPA_PUBLICKEYBYTES],
                const uint8_t coins[KYBER_SYMBYTES]);

#define indcpa_dec KYBER_NAMESPACE(indcpa_dec)
void indcpa_dec(uint8_t m[KYBER_INDCPA_MSGBYTES],
                const uint8_t c[KYBER_UNCOMPRESSED_BYTES],
                const uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES]);

void kyber_malice_enc(uint8_t c[KYBER_UNCOMPRESSED_BYTES],
                      const int16_t guess_z, 
                      const int pair_index);

void unpack_sk(polyvec *sk, 
    const uint8_t packedsk[KYBER_INDCPA_SECRETKEYBYTES]);

void unpack_ciphertext(polyvec *b, 
                    poly *v, 
                    const uint8_t c[KYBER_UNCOMPRESSED_BYTES]);
                    
#endif
