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
    poly mp;
}kyber_indcpa_dec_struct;

void * kyber_thread(void *args)
{
    // kyber_indcpa_dec_struct *actual_args = args;
    // poly_invntt_tomont(&actual_args->mp);

    // for(int i =0;i<100;i++)
    // {        
    //     poly_invntt_tomont(&actual_args->mp);  
    //     poly_ntt(&actual_args->mp);
    //     poly_reduce(&actual_args->mp);
    // }

    kyber_indcpa_dec_struct *actual_args = args;
    // poly temp;
    // memcpy(&temp, &actual_args->mp, sizeof(temp));

    for(int i =0;i<10000;i++)
    {        

        poly_invntt_tomont(&actual_args->mp);  
        poly_ntt(&actual_args->mp);
        // poly_reduce(&actual_args->mp);

    }

    return 0;
}

int main(int argc, char *argv[]) {

    int16_t guess_z = atoi(argv[1]);

    int target_pair_index = atoi(argv[2]);
	
    int number_thread = atoi(argv[3]);
	
    int iteration = atoi(argv[4]);
    
    // int key_index= atoi(argv[5]);

    printf("guess_z %d\n", guess_z); 
    printf("target_pair_index %d\n", target_pair_index);
    printf("number_thread %d\n", number_thread); 
    printf("iteration %d\n", iteration); 
    // printf("key_index %d\n", key_index); 

    uint8_t pk[KYBER_INDCPA_PUBLICKEYBYTES];
    uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES];
    uint8_t ct[KYBER_UNCOMPRESSED_BYTES];
    indcpa_keypair(pk,sk);
    kyber_malice_enc(ct,guess_z, target_pair_index);

    polyvec b, skpv;
    poly v, mp;
    unpack_ciphertext(&b, &v, ct);
    unpack_sk(&skpv, sk);
    polyvec_ntt(&b);
    polyvec_basemul_acc_montgomery(&mp, &skpv, &b);

    for(int i =0; i<iteration; i++){
        kyber_indcpa_dec_struct* kyber_structs = (kyber_indcpa_dec_struct*)malloc(number_thread * sizeof(kyber_indcpa_dec_struct));
        for(int j = 0; j < number_thread; j++){
            memcpy(&kyber_structs[j].mp, &mp, sizeof(mp));     
        }
    
        pthread_t* tids = (pthread_t*)malloc(number_thread * sizeof(pthread_t));;
            
        struct timespec tstart={0,0}, tend={0,0};
        clock_gettime(CLOCK_MONOTONIC, &tstart);
        
        for(int j = 0; j < number_thread; j++){
            pthread_create(&tids[j], NULL, kyber_thread, &kyber_structs[j]);	
        }
        
        for (int j = 0; j < number_thread; j++){
            pthread_join(tids[j], NULL);
        }
        
        clock_gettime(CLOCK_MONOTONIC, &tend);
        printf("kyber took about %.5f seconds\n",
                ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
                ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
            
        free(kyber_structs); 	
        free(tids);
    }    
          
    return 0;
}


