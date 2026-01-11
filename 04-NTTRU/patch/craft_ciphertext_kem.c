#include "craft_ciphertext_kem.h"
#include "craft_ciphertext.h"
#include "params.h"
#include "poly.h"
#include <string.h>

void crypto_kem_craft_ciphertext(unsigned char *c, int guess_z, int triple_index)
{
	poly chat;
	ntru_craft_ciphertext(&chat, guess_z, triple_index);
	poly_pack_uniform(c, &chat);
}
