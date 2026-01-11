#ifndef CRAFT_CIPHERTEXT_H
#define CRAFT_CIPHERTEXT_H

#include "poly.h"

// Craft a sparse ciphertext polynomial `chat` that places {1,1,z} at a
// triple location derived from `triple_index` in the AVX2 coefficient layout.
void ntru_craft_ciphertext(poly *chat, int guess_z, int triple_index);

#endif
