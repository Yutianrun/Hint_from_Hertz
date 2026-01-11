#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixed_key.h"
#include "kem.h"
#include "craft_ciphertext_kem.h"
#include "ntru.h"
#include "params.h"
#include "poly.h"

typedef struct {
	unsigned char c[CIPHERTEXTBYTES];
	unsigned char sk[SECRETKEYBYTES];
	unsigned char k[SHAREDKEYBYTES];
} kem_dec_ctx;

static void *kem_thread(void *args)
{
	kem_dec_ctx *ctx = (kem_dec_ctx *)args;
	while (1) {
		crypto_kem_dec(ctx->k, ctx->c, ctx->sk);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <guess_z> <triple_index> <threads>\n", argv[0]);
		return 1;
	}

	const int guess_z = atoi(argv[1]);
	const int triple_index = atoi(argv[2]);
	const int number_thread = atoi(argv[3]);

	unsigned char coins[N];
	nttru_fill_coins_fixed(coins);

	// Generate keypair
	unsigned char pk[PUBLICKEYBYTES];
	unsigned char sk[SECRETKEYBYTES];
	poly hhat, fhat;
	ntru_keygen(&hhat, &fhat, coins);

	// Pack keys in KEM format
	poly_pack_uniform(pk, &hhat);
	poly_pack_uniform(sk, &fhat);
	for (int i = 0; i < PUBLICKEYBYTES; ++i)
		sk[i + POLY_PACKED_UNIFORM_BYTES] = pk[i];

	// Create crafted ciphertext
	unsigned char c[CIPHERTEXTBYTES];
	crypto_kem_craft_ciphertext(c, guess_z, triple_index);

	// Allocate contexts
	kem_dec_ctx *ctx = NULL;
	const size_t ctx_size = (size_t)number_thread * sizeof(kem_dec_ctx);
	int rc = posix_memalign((void **)&ctx, 32, ctx_size);
	if (rc != 0) {
		errno = rc;
		perror("posix_memalign(ctx)");
		return 1;
	}

	pthread_t *tids = (pthread_t *)malloc((size_t)number_thread * sizeof(pthread_t));
	if (!tids) {
		perror("malloc");
		free(ctx);
		return 1;
	}

	for (int i = 0; i < number_thread; i++) {
		memcpy(ctx[i].c, c, CIPHERTEXTBYTES);
		memcpy(ctx[i].sk, sk, SECRETKEYBYTES);
		pthread_create(&tids[i], NULL, kem_thread, &ctx[i]);
	}

	for (int i = 0; i < number_thread; i++) {
		pthread_join(tids[i], NULL);
	}

	free(tids);
	free(ctx);
	return 0;
}
