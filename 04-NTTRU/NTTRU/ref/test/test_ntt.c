#include <stdint.h>
#include <stdio.h>
#include "../params.h"
#include "../fq.h"
#include "../ntt.h"

#define NTESTS 1000

int main(void) {
  unsigned int i, j;
  int16_t __attribute__((aligned(32))) a[768], b[768], c[768];
  
  init_ntt();
  memset(a,0,sizeof(a));
  memset(b,0,sizeof(b));
  memset(c,0,sizeof(c));
  // for(i = 0; i < NTESTS; ++i) {
  //   for(j = 0; j < 768; ++j)
  //     a[j] = fquniform();

  //   baseinv(b, a, 20*MONT % Q);
  //   basemul(c, a, b, 20*MONT % Q);
  //   c[0] = fqmul(c[0], 1);
  //   c[1] = fqmul(c[1], 1);
  //   c[2] = fqmul(c[2], 1);
  //   if((c[0] - 1) % Q || c[1] % Q || c[2] % Q)
  //     printf("Failure in baseinv/basemul: c = %hd + %hd*X + %hd*X^2\n",
  //            c[0], c[1], c[2]);

  //   ntt(b, a);
  //   invntt(c, b);
  //   for(j = 0; j < 768; ++j)
  //     if((fqmul(c[j], 1) - a[j]) % Q)
  //       printf("Failure in ntt/invntt: c[%u] = %hd != %hd\n",
  //              j, fqmul(c[j], 1), a[j]);
  // }


  a[0]=18;
  for (size_t i = 0; i < 768; i++)
  {
    /* code */
    printf("%d,",a[i]);
  }
  printf("\n");
  
  invntt(b,a);

  for (size_t i = 0; i < 768; i++)
  {
    /* code */
    b[i]=b[i]*900%7681;   //900为mont的逆
  }


  ntt(c,b);
  for (size_t i = 0; i < 768; i++)
  {
    /* code */
    printf("%d,",c[i]);
  }
  printf("\n");

  



  return 0;
}
