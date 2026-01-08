#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

#include "indcpa.h"
#include "poly.h"
#include "polyvec.h"
#include "craft_ciphertext.h"

static void indcpa_keypair_local(uint8_t pk[KYBER_INDCPA_PUBLICKEYBYTES],
                                 uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES])
{
  uint8_t coins[KYBER_SYMBYTES] = {8};

  indcpa_keypair_derand(pk, sk, coins);
}

static void print_hex(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    printf("%02x", buf[i]);
  }
}

int main(int argc, char **argv) {
  int16_t z1 = 1;
  int16_t z2 = 2246;
  int pair_index = 0;

  if (argc >= 2 && strcmp(argv[1], "--scan") == 0) {
    if (argc == 3) {
      pair_index = atoi(argv[2]);
    } else if (argc != 2) {
      fprintf(stderr, "Usage: %s --scan [pair_index]\n", argv[0]);
      return 1;
    }
  } else if (argc == 4) {
    z1 = (int16_t)atoi(argv[1]);
    z2 = (int16_t)atoi(argv[2]);
    pair_index = atoi(argv[3]);
  } else if (argc != 1) {
    fprintf(stderr, "Usage: %s [z1 z2 pair_index]\n", argv[0]);
    fprintf(stderr, "       %s --scan [pair_index]\n", argv[0]);
    return 1;
  }

  uint8_t pk[KYBER_INDCPA_PUBLICKEYBYTES];
  uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES];
  indcpa_keypair_local(pk, sk);

  uint8_t sk_sha256[SHA256_DIGEST_LENGTH];
  SHA256(sk, sizeof(sk), sk_sha256);
  printf("sk_sha256=");
  print_hex(sk_sha256, sizeof(sk_sha256));
  printf("\n");

  if (argc >= 2 && strcmp(argv[1], "--scan") == 0) {
    polyvec skpv;
    polyvec_frombytes(&skpv, sk);

    int hits = 0;
    for (int z = 0; z < KYBER_Q; z++) {
      uint8_t ct_patch[KYBER_UNCOMPRESSED_BYTES];
      craft_ciphertext(ct_patch, 1, (int16_t)z, pair_index);

      polyvec b_dec;
      poly v_dec;
      poly mp;
      unpack_ciphertext_uncompressed(&b_dec, &v_dec, ct_patch);

      polyvec_ntt(&b_dec);
      polyvec_basemul_acc_montgomery(&mp, &skpv, &b_dec);
      poly_invntt_tomont(&mp);

      int modq_zero = 0;
      for (int i = 0; i < KYBER_N; i++) {
        int vq = mp.coeffs[i] % KYBER_Q;
        if (vq < 0)
          vq += KYBER_Q;
        if (vq == 0)
          modq_zero++;
      }

      if (modq_zero == KYBER_N / 2) {
        printf("half_zero_z=%d\n", z);
        hits++;
      }
    }

    fprintf(stderr, "scan_done hits=%d\n", hits);
    return hits == 2 ? 0 : 2;
  }

  uint8_t ct_patch[KYBER_UNCOMPRESSED_BYTES];
  craft_ciphertext(ct_patch, z1, z2, pair_index);

  polyvec b_dec, skpv;
  poly v_dec, mp;
  unpack_ciphertext_uncompressed(&b_dec, &v_dec, ct_patch);
  polyvec_frombytes(&skpv, sk);

  polyvec_ntt(&b_dec);
  polyvec_basemul_acc_montgomery(&mp, &skpv, &b_dec);
  poly_invntt_tomont(&mp);

  int exact_zero = 0;
  int modq_zero = 0;
  for (int i = 0; i < KYBER_N; i++) {
    const int16_t c = mp.coeffs[i];
    if (c == 0)
      exact_zero++;

    int vq = c % KYBER_Q;
    if (vq < 0)
      vq += KYBER_Q;
    if (vq == 0)
      modq_zero++;
  }
  printf("mp_after_invntt_zero_exact=%d/%d\n", exact_zero, KYBER_N);
  printf("mp_after_invntt_zero_modq=%d/%d\n", modq_zero, KYBER_N);

  const int half = KYBER_N / 2;
  const int half_zero_ok = (exact_zero == half) || (modq_zero == half);
  printf("mp_after_invntt_half_zero_ok=%s\n", half_zero_ok ? "yes" : "no");
  if (!half_zero_ok)
    return 3;
  return 0;
}
