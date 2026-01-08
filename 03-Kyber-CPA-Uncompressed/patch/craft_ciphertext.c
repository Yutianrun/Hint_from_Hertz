/*
 * Craft uncompressed ciphertext for frequency side-channel attack.
 * Uses modified invntt with scaling factor 512 (instead of 1441).
 */

#include <stdint.h>
#include <string.h>
#include "craft_ciphertext.h"
#include "poly.h"

#define KYBER_Q 3329
#define QINV (-3327)  /* q^-1 mod 2^16 */

static const int16_t zetas[128] = {
  -1044,  -758,  -359, -1517,  1493,  1422,   287,   202,
   -171,   622,  1577,   182,   962, -1202, -1474,  1468,
    573, -1325,   264,   383,  -829,  1458, -1602,  -130,
   -681,  1017,   732,   608, -1542,   411,  -205, -1571,
   1223,   652,  -552,  1015, -1293,  1491,  -282, -1544,
    516,    -8,  -320,  -666, -1618, -1162,   126,  1469,
   -853,   -90,  -271,   830,   107, -1421,  -247,  -951,
   -398,   961, -1508,  -725,   448, -1065,   677, -1275,
  -1103,   430,   555,   843, -1251,   871,  1550,   105,
    422,   587,   177,  -235,  -291,  -460,  1574,  1653,
   -246,   778,  1159,  -147,  -777,  1483,  -602,  1119,
  -1590,   644,  -872,   349,   418,   329,  -156,   -75,
    817,  1097,   603,   610,  1322, -1285, -1465,   384,
  -1215,  -136,  1218, -1335,  -874,   220, -1187, -1659,
  -1185, -1530, -1278,   794, -1510,  -854,  -870,   478,
   -108,  -308,   996,   991,   958, -1460,  1522,  1628
};

static int16_t barrett_reduce(int16_t a) {
    int16_t t;
    const int16_t v = ((1 << 26) + KYBER_Q / 2) / KYBER_Q;
    t = ((int32_t)v * a + (1 << 25)) >> 26;
    t *= KYBER_Q;
    return a - t;
}

static int16_t montgomery_reduce(int32_t a) {
    int16_t t;
    t = (int16_t)a * QINV;
    t = (a - (int32_t)t * KYBER_Q) >> 16;
    return t;
}

static int16_t fqmul(int16_t a, int16_t b) {
    return montgomery_reduce((int32_t)a * b);
}

/* invntt with scaling factor 512 (for crafted ciphertext) */
static void craft_invntt(int16_t r[256]) {
    unsigned int start, len, j, k;
    int16_t t, zeta;
    const int16_t f = 512;  /* scaling factor for crafted ciphertext */

    k = 127;
    for (len = 2; len <= 128; len <<= 1) {
        for (start = 0; start < 256; start = j + len) {
            zeta = zetas[k--];
            for (j = start; j < start + len; j++) {
                t = r[j];
                r[j] = barrett_reduce(t + r[j + len]);
                r[j + len] = r[j + len] - t;
                r[j + len] = fqmul(zeta, r[j + len]);
            }
        }
    }

    for (j = 0; j < 256; j++)
        r[j] = fqmul(r[j], f);
}

/* Apply craft_invntt to all polynomials in polyvec */
static void craft_polyvec_invntt(polyvec *r) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++)
        craft_invntt(r->vec[i].coeffs);
}

/* Pack uncompressed ciphertext (no compression) */
static void pack_ciphertext_uncompressed(uint8_t r[KYBER_UNCOMPRESSED_BYTES],
                                         polyvec *b, poly *v) {
    memcpy(r, b, KYBER_POLYVECUNCOMPRESSED);
    memcpy(r + KYBER_POLYVECUNCOMPRESSED, v, KYBER_POLYUNCOMPRESSED);
}

void unpack_ciphertext_uncompressed(polyvec *b, poly *v,
                                    const uint8_t c[KYBER_UNCOMPRESSED_BYTES]) {
    memcpy(b, c, KYBER_POLYVECUNCOMPRESSED);
    memcpy(v, c + KYBER_POLYVECUNCOMPRESSED, KYBER_POLYUNCOMPRESSED);
}

/*
 * Craft ciphertext with specific z values at given pair index.
 * Sets b[0].coeffs[2*pair_index] = z1, b[0].coeffs[2*pair_index+1] = z2
 * then applies invntt to create the uncompressed ciphertext.
 */
void craft_ciphertext(uint8_t c[KYBER_UNCOMPRESSED_BYTES],
                      int16_t z1, int16_t z2, int pair_index) {
    polyvec b;
    poly v;

    memset(&b, 0, KYBER_POLYVECUNCOMPRESSED);
    memset(&v, 0, KYBER_POLYUNCOMPRESSED);

    b.vec[0].coeffs[2 * pair_index] = z1;
    b.vec[0].coeffs[2 * pair_index + 1] = z2;

    craft_polyvec_invntt(&b);
    pack_ciphertext_uncompressed(c, &b, &v);
}

/*
 * Decrypt uncompressed ciphertext.
 * This is indcpa_dec modified to work with uncompressed ciphertext.
 */
void indcpa_dec_uncompressed(uint8_t m[32],
                             const uint8_t c[KYBER_UNCOMPRESSED_BYTES],
                             const uint8_t sk[KYBER_POLYVECBYTES]) {
    polyvec b, skpv;
    poly v, mp;

    unpack_ciphertext_uncompressed(&b, &v, c);
    polyvec_frombytes(&skpv, sk);

    polyvec_ntt(&b);
    polyvec_basemul_acc_montgomery(&mp, &skpv, &b);
    poly_invntt_tomont(&mp);

    poly_sub(&mp, &v, &mp);
    poly_reduce(&mp);

    poly_tomsg(m, &mp);
}
