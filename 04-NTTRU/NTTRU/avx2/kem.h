#ifndef KEM_H
#define KEM_H

#include "params.h"

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk);
int crypto_kem_enc(unsigned char *c,
                   unsigned char *k,
                   const unsigned char *pk);
int crypto_kem_dec(unsigned char *k,
                   const unsigned char *c,
                   const unsigned char *sk);

int crypto_kem_malice_enc(unsigned char *c, 
                          int16_t guess_z, 
                          int triplle_index);
int crypto_kem_malice_enc1(unsigned char *c, 
                          int16_t guess_z1,
                          int16_t guess_z2,  
                          int triplle_index);                          
#endif
