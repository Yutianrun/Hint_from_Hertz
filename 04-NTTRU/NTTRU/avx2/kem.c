#include <stddef.h>
#include <openssl/sha.h>
#include "randombytes.h"
#include "crypto_stream.h"
#include "params.h"
#include "ntru.h"
#include "poly.h"

#include "../ref/ntt.h"

static const unsigned char n[16] = {0};

int crypto_kem_keypair(unsigned char *pk, unsigned char *sk) {
  unsigned int i;
  unsigned char coins[N];
  poly hhat, fhat;

  do {
    randombytes(coins, 32);
    crypto_stream(coins, N, n, coins);
  } while(ntru_keygen(&hhat, &fhat, coins));

  poly_pack_uniform(pk, &hhat);
  poly_pack_uniform(sk, &fhat);
  for(i = 0; i < PUBLICKEYBYTES; ++i)
    sk[i + POLY_PACKED_UNIFORM_BYTES] = pk[i];

  return 0;
}

int crypto_kem_enc(unsigned char *c, unsigned char *k, const unsigned char *pk) {
  unsigned int i;
  unsigned char buf[32 + COINBYTES];
  poly hhat, chat, m;

  randombytes(buf, 32);
  crypto_stream(buf, N/2, n, buf);
  poly_short(&m, buf);
  poly_pack_short(buf, &m);
  SHA512(buf, MSGBYTES, buf);
  crypto_stream(buf + 32, COINBYTES, n, buf + 32);

  poly_unpack_uniform(&hhat, pk);
  ntru_encrypt(&chat, &hhat, &m, buf + 32);
  poly_pack_uniform(c, &chat);

  for (i = 0; i < SHAREDKEYBYTES; ++i)
    k[i] = buf[i];

  return 0;
}
int crypto_kem_malice_enc(unsigned char *c, 
                          int16_t guess_z, 
                          int triplle_index) {
  poly chat;
  memset(&chat,0,sizeof(chat));
  int ref_index = triplle_index * 3;
  int avx_index = (ref_index / 96) * 96 + (ref_index % 96) / 6 + (ref_index % 96 % 6) * 16;
  chat.coeffs[avx_index] = 1;
  chat.coeffs[avx_index + 16] = 1;
  chat.coeffs[avx_index + 32] = guess_z;
  poly_reduce(&chat);
  poly_pack_uniform(c, &chat);
  return 0;
}

int crypto_kem_malice_enc_test(unsigned char *c, 
                          int16_t guess_z, 
                          int triplle_index) {

  // int16_t temp[768]={0};
  // int16_t temp_t[768]={0};
  poly chat;
  memset(&chat,0,sizeof(chat));
  // temp[triplle_index*3+0]=1;
  // temp[triplle_index*3+1]=1;
  // temp[triplle_index*3+2]=guess_z; 
  // invntt(temp_t,temp);
  // for (size_t i = 0; i < 768; i++)
  // {
  //   /* code */
  //   temp_t[i]=temp_t[i]*900%7681;   // 900 is the modular inverse for the Montgomery factor
  // }






  int ref_index = triplle_index * 3;
  int avx_index = (ref_index / 96) * 96 + (ref_index % 96) / 6 + (ref_index % 96 % 6) * 16;
  chat.coeffs[avx_index] = 1;
  chat.coeffs[avx_index + 16] = 1;
  chat.coeffs[avx_index + 32] = guess_z;
  poly_reduce(&chat);
  poly_pack_uniform(c, &chat);
  return 0;
}

int crypto_kem_malice_enc1(unsigned char *c, 
                          int16_t guess_z1,
                          int16_t guess_z2,  
                          int triplle_index) {
  poly chat;
  memset(&chat,0,sizeof(chat));
  int ref_index = triplle_index * 3;
  int avx_index = (ref_index / 96) * 96 + (ref_index % 96) / 6 + (ref_index % 96 % 6) * 16;
  chat.coeffs[avx_index] = 1;
  chat.coeffs[avx_index + 16] = guess_z1;
  chat.coeffs[avx_index + 32] = guess_z2;
  poly_reduce(&chat);
  poly_pack_uniform(c, &chat);
  return 0;
}
int crypto_kem_dec(unsigned char *k,
                   const unsigned char *c,
                   const unsigned char *sk)
{
  unsigned int i;
  unsigned char buf[32 + COINBYTES];
  int16_t t;
  int32_t fail;
  poly m, hhat, chat, fhat;

  poly_unpack_uniform(&chat, c);
  poly_unpack_uniform(&fhat, sk);
  ntru_decrypt(&m, &chat, &fhat);

  poly_pack_short(buf, &m);
  SHA512(buf, MSGBYTES, buf);
  crypto_stream(buf + 32, COINBYTES, n, buf + 32);

  poly_unpack_uniform(&hhat, sk + POLY_PACKED_UNIFORM_BYTES);
  ntru_encrypt(&fhat, &hhat, &m, buf + 32);

  t = 0;
  for(i = 0; i < N; ++i)
    t |= chat.coeffs[i] ^ fhat.coeffs[i];

  fail = (uint16_t)t;
  fail = (-fail) >> 31;
  for(i = 0; i < SHAREDKEYBYTES; ++i)
    k[i] = buf[i] & ~(-fail);

  return fail;
}
