#include <string.h>

#include "craft_ciphertext.h"
#include "params.h"

void ntru_craft_ciphertext(poly *chat, int guess_z, int triple_index)
{
	poly chat_temp;
	memset(chat, 0, sizeof(chat_temp));

	// NOTE: This mapping matches the experiment code used in the original repo.
	// `triple_index` is interpreted in the AVX2 coefficient layout.
	const int avx_index = triple_index / 16 + (triple_index % 16) / 3;

	chat->coeffs[avx_index] = 1;
	chat->coeffs[avx_index + 16] = 1;
	chat->coeffs[avx_index + 32] = (int16_t)guess_z;

	poly_reduce(chat);
}
