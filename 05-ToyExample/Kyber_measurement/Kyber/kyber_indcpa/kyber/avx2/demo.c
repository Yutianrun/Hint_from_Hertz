#include <stdio.h>
#include <string.h>
#include "poly.h"
int main(int argc, char *argv[]) {
    poly a,b,res;
    memset(&a,0,sizeof(a));
    memset(&b,0,sizeof(b));
    memset(&res,0,sizeof(res));

    // poly a: 4x^2+2x+6
    // a.coeffs[0]=3000;
    // a.coeffs[1]=2522;
    // a.coeffs[2]=4;
    // a.coeffs[62]=622;
    for (size_t i = 0; i < 128; i++)
    {
        /* code */
        a.coeffs[i]=i+1;
    }
    
    poly_ntt(&a);

    // a.coeffs[0]=1;
    // poly_ntt(&a);
    // a.coeffs[32]=3;
    // a.coeffs[2]=2;
    // poly b: 2x+3
    // b.coeffs[0]=3;
    // b.coeffs[1]=2;
    // b.coeffs[90]=512;
    for (size_t i = 0; i < 128; i++)
    {
        /* code */
        b.coeffs[i]=i+1;
    }
    poly_ntt(&b);
    // poly_invntt_tomont(&a);

    // right res: 8x^3+16x^2+18x+18
    poly_basemul_montgomery(&res,&a,&b);
    poly_invntt_tomont(&res);
    poly_reduce(&res);

    printf("Resulting product res:\n");
    for (size_t i = 0; i < 128; i++)
    {
        /* code */
        printf("%d,",res.coeffs[i]);
    }
    printf("\n");
}
