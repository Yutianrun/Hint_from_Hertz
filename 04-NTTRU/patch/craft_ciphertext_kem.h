#ifndef CRAFT_CIPHERTEXT_KEM_H
#define CRAFT_CIPHERTEXT_KEM_H

// Pack a crafted ciphertext for KEM decapsulation
void crypto_kem_craft_ciphertext(unsigned char *c, int guess_z, int triple_index);

#endif
