#include "randombytes.h"
#include "params.h"
#include "poly.h"
#include "string.h"
int ntru_keygen(poly *hhat, poly *fhat, const unsigned char coins[N]) {
  int r;
  poly f, g;

  poly_short(&f, coins);
  poly_short(&g, coins + N/2);
  poly_triple(&g, &g);
  poly_triple(&f, &f);
  f.coeffs[0] += 1;
  poly_ntt(fhat, &f);
  poly_ntt(&g, &g);
  poly_freeze(fhat);
  r = poly_baseinv(&f, fhat);
  poly_basemul(hhat, &f, &g);
  poly_freeze(hhat);
  return r;
}

void ntru_encrypt(poly *chat,
                  const poly *hhat,
                  const poly *m,
                  const unsigned char coins[N/2])
{
  poly r, mhat;

  poly_short(&r, coins);
  poly_ntt(&r, &r);
  poly_ntt(&mhat, m);
  poly_basemul(chat, &r, hhat);
  poly_reduce(chat);
  poly_add(chat, chat, &mhat);
  poly_freeze(chat);
}

void ntru_decrypt(poly *m,
                  const poly *chat,
                  const poly *fhat)
{
  poly_basemul(m, chat, fhat);
  poly_reduce(m);
  poly_invntt(m, m);

  // int count=0;
  // printf("normal(m):\n");
  // for (size_t i = 0; i < 768; i++)
  // {
  //   /* code */
  //   // printf("%d,",m->coeffs[i]);
  //   if(m->coeffs[i]!=0)
  //     count++;
  // }
  // if(count<700){ printf("hit\n");}
  // printf("count:\n%d\n",count);

  poly_crepmod3(m, m);
  // for (size_t i = 0; i < 768; i++)
  // {
  //   /* code */
  //   printf("%d,",m->coeffs[i]);
  //   // if(m->coeffs[i]!=0)
  //     // count++;
  // }
  // printf("\n");

}

void ntru_malice_enc(poly *chat, int guess_z, int triplle_index)
{
  poly chat_temp;
  memset(chat, 0, sizeof(chat_temp));

  int avx_index = triplle_index/16 + (triplle_index%16)/3;
  chat->coeffs[avx_index] = 1;
  chat->coeffs[avx_index+16] = 1;
  chat->coeffs[avx_index+32] = guess_z;
  poly_reduce(chat);
}

void ntru_malice_enc_two_pair(poly *chat, int guess_z, int triplle_index)
{
  poly chat_temp;
  memset(chat, 0, sizeof(chat_temp));

  
  int avx_index = triplle_index/16 + (triplle_index%16)/3;
  chat->coeffs[0] = 1;
  chat->coeffs[16] = 1;
  chat->coeffs[32] = 942;

  chat->coeffs[avx_index] = 1;
  chat->coeffs[avx_index+16] = 1;
  chat->coeffs[avx_index+32] = guess_z;
  poly_reduce(chat);
}