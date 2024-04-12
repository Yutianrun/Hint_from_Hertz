#ifndef NTRU_H
#define NTRU_H

#include "poly.h"

int ntru_keygen(poly *hhat, poly *fhat, const unsigned char *coins);
void ntru_encrypt(poly *chat,
                  const poly *hhat,
                  const poly *m,
                  const unsigned char *coins);
void ntru_decrypt(poly *m,
                  const poly *chat,
                  const poly *fhat);

void ntru_malice_enc(poly *chat ,
                    int16_t guess_z, 
                    int triplle_index);

void ntru_malice_enc_two_pair(poly *chat ,
                    int16_t guess_z, 
                    int triplle_index);
#endif
