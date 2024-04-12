#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include "kem.h"
#include "randombytes.h"
#include "indcpa.h"
#include "polyvec.h"
#include "polyvec.h"


typedef struct {
    uint8_t m[KYBER_INDCPA_MSGBYTES];
    uint8_t c[KYBER_UNCOMPRESSED_BYTES];
    uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES];
}kyber_indcpa_dec_struct;

void * kyber_thread(void *args)
{
    kyber_indcpa_dec_struct *actual_args = args;
    while(1)
    // for(int i =0 ;i<10000;i++)
    indcpa_dec(actual_args->m,actual_args->c,actual_args->sk);
    return 0;
}

int main(int argc, char *argv[]) {

    int16_t guess_z = atoi(argv[1]);

    int target_pair_index = atoi(argv[2]);
	
    int number_thread = atoi(argv[3]);
	
    int iteration = atoi(argv[4]);
    
    // int key_index= atoi(argv[5]);

    // poly test={{2983,2155,3170,105,904,2564,2439,348,1848,3327,3272,971,2991,3236,2815,3068,164,879,3291,143,2829,833,188,1389,2337,2545,2407,2834,186,453,2057,2009,456,1258,1264,884,279,122,459,1934,1475,551,3199,2419,803,1079,1229,3126,1035,2280,1732,853,2990,1869,2247,3073,2347,302,2636,2325,2685,2662,1770,937,1371,3284,559,980,902,260,2147,2834,529,2281,1607,604,3016,1482,618,2769,3322,2434,2358,3224,305,2129,2264,2169,1019,956,2154,100,2776,2468,1917,219,288,742,590,422,1996,478,1742,731,67,2530,1564,750,2961,1231,2219,318,2851,2913,669,259,2321,2120,907,3321,1906,2914,107,1436,33,3078,101,2149}};
    // test.coeffs[0]=1;
    // for (size_t i = 1; i < 128; i++)
    // {
    //     /* code */
    //     test.coeffs[i]=0;
    // }
    // poly_invntt_tomont(&test);
    // printf("invntt:test ");
    // for (size_t i = 0; i < 128; i++)
    // {
    //     /* code */
    //     printf("%d,", test.coeffs[i]);
    // }
    // printf("\n");

    // printf("guess_z %d\n", guess_z); 
    // printf("target_pair_index %d\n", target_pair_index);
    // printf("number_thread %d\n", number_thread); 
    // printf("iteration %d\n", iteration); 
    // printf("key_index %d\n", key_index); 

    uint8_t pk[KYBER_INDCPA_PUBLICKEYBYTES];
    uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES]={167,75,10,200,177,64,91,165,207,32,49,178,107,248,54,234,132,142,212,44,152,230,18,182,98,188,205,240,68,108,47,98,147,78,210,41,105,240,8,116,83,53,212,131,201,166,49,16,136,211,176,23,225,186,134,19,19,204,23,145,4,26,52,122,208,116,4,17,133,222,129,132,135,201,11,203,113,140,99,136,141,206,182,56,92,209,86,142,23,192,18,155,135,219,146,207,56,23,146,195,181,146,17,178,63,67,32,119,255,28,159,39,226,18,233,200,59,226,41,182,200,124,150,127,204,164,71,166,134,28,182,6,203,35,177,115,89,145,92,66,6,238,194,89,175,171,11,35,211,167,200,139,173,145,27,2,164,92,28,55,100,166,202,69,154,207,100,192,255,154,128,205,164,110,106,210,119,171,88,6,252,155,125,54,156,58,209,186,13,62,81,134,19,7,200,254,35,135,180,108,100,205,67,147,81,218,63,110,243,152,109,84,58,183,187,115,32,4,160,106,132,154,174,83,193,117,65,206,198,227,135,141,97,154,203,73,170,184,230,81,204,129,26,7,165,114,52,236,62,134,23,76,18,210,104,62,230,141,11,24,185,233,40,203,158,160,119,150,130,175,104,166,120,184,102,109,34,164,88,70,38,118,222,229,175,125,156,73,204,7,162,167,54,167,71,49,143,40,65,44,35,104,24,248,188,11,68,106,29,247,124,58,34,28,193,119,188,99,132,19,15,82,112,192,93,68,188,201,218,154,156,19,109,150,12,199,71,88,166,33,75,38,95,160,180,63,195,80,143,245,139,248,100,17,14,217,33,159,198,28,176,89,88,64,197,151,132,227,46,124,72,69,117,36,35,112,20,96,4,194,69,206,49,121};
    // uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES];
    uint8_t ct[KYBER_UNCOMPRESSED_BYTES];

    // indcpa_keypair(pk,sk);
    // printf("zzz \n"); 
    kyber_malice_enc(ct,guess_z, target_pair_index);
    kyber_indcpa_dec_struct *kyber_structs = (kyber_indcpa_dec_struct *)malloc(number_thread * sizeof(kyber_indcpa_dec_struct));
    for (int j = 0; j < number_thread; j++)
    {
        memcpy(&kyber_structs[j].c, &ct, sizeof(ct));
        memcpy(&kyber_structs[j].sk, &sk, sizeof(sk));
    }

    pthread_t *tids = (pthread_t *)malloc(number_thread * sizeof(pthread_t));
    ;

    struct timespec tstart = {0, 0}, tend = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    for(int i =0; i<iteration; i++){
        
        
        for(int j = 0; j < number_thread; j++){
            pthread_create(&tids[j], NULL, kyber_thread, &kyber_structs[j]);	
        }
        
        for (int j = 0; j < number_thread; j++){
            pthread_join(tids[j], NULL);
        }
        
    }
    clock_gettime(CLOCK_MONOTONIC, &tend);
    // printf("kyber took about %.5f seconds\n",
    //         ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
    //         ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));

    free(kyber_structs);
    free(tids);

    return 0;
}


