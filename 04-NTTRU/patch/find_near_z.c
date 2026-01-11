// White-box verification tool: find near-z values using known secret key
// Used to verify which z values minimize HW after invntt(chat*fhat)

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
	int z;
	int hw3;
	int invntt_nonzero;
	long long sum_abs;
} score_t;

static long long iabsll(long long x) { return x < 0 ? -x : x; }

static void maybe_insert(score_t *best, int k, score_t cand)
{
	for (int i = 0; i < k; i++) {
		int better = 0;
		if (cand.hw3 < best[i].hw3)
			better = 1;
		else if (cand.hw3 == best[i].hw3 && cand.invntt_nonzero < best[i].invntt_nonzero)
			better = 1;
		else if (cand.hw3 == best[i].hw3 && cand.invntt_nonzero == best[i].invntt_nonzero && cand.sum_abs < best[i].sum_abs)
			better = 1;

		if (!better)
			continue;

		for (int j = k - 1; j > i; j--)
			best[j] = best[j - 1];
		best[i] = cand;
		return;
	}
}

static void usage(const char *prog)
{
	fprintf(stderr,
	        "Usage: %s [-i triple_index] [-k topk] [-q]\n"
	        "\n"
	        "Heuristic: brute-force z in [0,Q) and rank by\n"
	        "  1) Hamming weight after crepmod3(invntt(chat*fhat))\n"
	        "  2) #nonzero coeffs after invntt(chat*fhat)\n"
	        "  3) sum(abs(coeffs)) as tie-breaker\n"
	        "\n"
	        "Output:\n"
	        "  default: verbose table + ZLIST line\n"
	        "  -q: only prints space-separated z list\n",
	        prog);
}

int main(int argc, char **argv)
{
	int triple_index = 0;
	int topk = 3;
	int quiet = 0;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-i") && i + 1 < argc) {
			triple_index = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-k") && i + 1 < argc) {
			topk = atoi(argv[++i]);
		} else if (!strcmp(argv[i], "-q")) {
			quiet = 1;
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			usage(argv[0]);
			return 0;
		} else {
			usage(argv[0]);
			return 2;
		}
	}

	if (topk <= 0 || topk > 64) {
		fprintf(stderr, "topk must be in [1,64]\n");
		return 2;
	}

	unsigned char coins[N];
	nttru_fill_coins_fixed(coins);

	poly hhat, fhat;
	ntru_keygen(&hhat, &fhat, coins);

	score_t best[64];
	for (int i = 0; i < topk; i++) {
		best[i].z = -1;
		best[i].hw3 = 1 << 30;
		best[i].invntt_nonzero = 1 << 30;
		best[i].sum_abs = (1LL << 62);
	}

	for (int z = 0; z < Q; z++) {
		poly chat, m, m3;
		ntru_craft_ciphertext(&chat, z, triple_index);

		poly_basemul(&m, &chat, &fhat);
		poly_reduce(&m);
		poly_invntt(&m, &m);

		int invntt_nonzero = 0;
		long long sum_abs = 0;
		for (int i = 0; i < N; i++) {
			const int16_t v = m.coeffs[i];
			if (v != 0)
				invntt_nonzero++;
			sum_abs += iabsll((long long)v);
		}

		poly_crepmod3(&m3, &m);
		int hw3 = 0;
		for (int i = 0; i < N; i++) {
			if (m3.coeffs[i] != 0)
				hw3++;
		}

		score_t cand = {.z = z, .hw3 = hw3, .invntt_nonzero = invntt_nonzero, .sum_abs = sum_abs};
		maybe_insert(best, topk, cand);
	}

	if (quiet) {
		for (int i = 0; i < topk; i++) {
			if (i)
				printf(" ");
			printf("%d", best[i].z);
		}
		printf("\n");
		return 0;
	}

	printf("triple_index=%d Q=%d N=%d\n", triple_index, Q, N);
	printf("Ranked by: hw3 asc, invntt_nonzero asc, sum_abs asc\n\n");
	printf("%5s %10s %23s\n", "rank", "z", "hw3/invntt_nonzero/sum_abs");
	for (int i = 0; i < topk; i++) {
		printf("%5d %10d %5d / %5d / %lld\n", i + 1, best[i].z, best[i].hw3, best[i].invntt_nonzero, best[i].sum_abs);
	}
	printf("\nZLIST:");
	for (int i = 0; i < topk; i++) {
		printf(" %d", best[i].z);
	}
	printf("\n");

	return 0;
}
