#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fixed_key.h"
#include "craft_ciphertext.h"
#include "ntru.h"
#include "params.h"
#include "poly.h"

typedef struct {
	poly m;
	poly chat;
	poly fhat;
} nttru_dec_ctx;

static void *nttru_thread(void *args)
{
	nttru_dec_ctx *ctx = (nttru_dec_ctx *)args;
	while (1) {
		ntru_decrypt(&ctx->m, &ctx->chat, &ctx->fhat);
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

	poly hhat, fhat, chat;
	ntru_keygen(&hhat, &fhat, coins);
	ntru_craft_ciphertext(&chat, guess_z, triple_index);

	// IMPORTANT: NTTRU AVX2 code uses aligned loads/stores; keep ctx 32B aligned.
	nttru_dec_ctx *ctx = NULL;
	const size_t ctx_size = (size_t)number_thread * sizeof(nttru_dec_ctx);
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
		memcpy(&ctx[i].chat, &chat, sizeof(chat));
		memcpy(&ctx[i].fhat, &fhat, sizeof(fhat));
		pthread_create(&tids[i], NULL, nttru_thread, &ctx[i]);
	}

	for (int i = 0; i < number_thread; i++) {
		pthread_join(tids[i], NULL);
	}

	free(tids);
	free(ctx);
	return 0;
}
