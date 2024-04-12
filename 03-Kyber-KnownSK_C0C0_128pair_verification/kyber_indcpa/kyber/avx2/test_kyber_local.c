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
#include "malice_invntt.h"

typedef struct {
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t ct[KYBER_UNCOMPRESSED_BYTES];
    uint8_t key[CRYPTO_BYTES];
}kyber_dec_struct;

void * kyber_thread(void *args)
{
    kyber_dec_struct *actual_args = args;
    for(int i =0; i<100000000;i++)
    crypto_kem_dec(actual_args->key,actual_args->ct,actual_args->sk);
    return 0;
}

int main(int argc, char *argv[]) {

    int hit_difference = atoi(argv[1]);

    int target_pair_index = atoi(argv[2]);
	
    int number_thread = atoi(argv[3]);
	
    int iteration = atoi(argv[4]);
    
    // int key_index= atoi(argv[5]);

    printf("hit_difference %d\n", hit_difference); 
    printf("target_pair_index %d\n", target_pair_index);
    printf("number_thread %d\n", number_thread); 
    printf("iteration %d\n", iteration); 
    // printf("key_index %d\n", key_index); 

    uint8_t pk[CRYPTO_PUBLICKEYBYTES];
    uint8_t sk[CRYPTO_SECRETKEYBYTES];
    uint8_t ct[CRYPTO_CIPHERTEXTBYTES];

    crypto_kem_keypair(pk,sk);

    kyber_malice_enc(ct,  hit_difference, target_pair_index);

    for(int i =0; i<iteration; i++){
        kyber_dec_struct* kyber_structs = (kyber_dec_struct*)malloc(number_thread * sizeof(kyber_dec_struct));
        for(int j = 0; j < number_thread; j++){
            memcpy(&kyber_structs[j].ct, &ct, sizeof(ct)); 
            memcpy(&kyber_structs[j].sk, &sk, sizeof(sk)); 	    
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


