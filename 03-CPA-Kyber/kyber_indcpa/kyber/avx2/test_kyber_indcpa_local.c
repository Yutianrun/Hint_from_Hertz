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
    // while(1)
    for(int i =0 ;i<100000;i++)
    indcpa_dec(actual_args->m,actual_args->c,actual_args->sk);
    return 0;
}

int main(int argc, char *argv[]) {

    int16_t guess_z1 = atoi(argv[1]);
    int16_t guess_z2 = atoi(argv[2]);

    int target_pair_index = atoi(argv[3]);
	
    int number_thread = atoi(argv[4]);
	
    int iteration = atoi(argv[5]);
    
    // int key_index= atoi(argv[5]);

    // printf("guess_z1 %d\n", guess_z1);
    // printf("guess_z2 %d\n", guess_z2);
    // printf("target_pair_index %d\n", target_pair_index);
    // printf("number_thread %d\n", number_thread);
    // printf("iteration %d\n", iteration); 
    // printf("key_index %d\n", key_index); 

    uint8_t pk[KYBER_INDCPA_PUBLICKEYBYTES];
    uint8_t sk[KYBER_INDCPA_SECRETKEYBYTES];
    uint8_t ct[KYBER_UNCOMPRESSED_BYTES];

    indcpa_keypair(pk,sk);

    kyber_malice_enc(ct,guess_z1, guess_z2, target_pair_index);

    for(int i =0; i<iteration; i++){
        kyber_indcpa_dec_struct* kyber_structs = (kyber_indcpa_dec_struct*)malloc(number_thread * sizeof(kyber_indcpa_dec_struct));
        for(int j = 0; j < number_thread; j++){
            memcpy(&kyber_structs[j].c, &ct, sizeof(ct)); 
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
        // printf("kyber took about %.5f seconds\n",
        //         ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
        //         ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
            
        free(kyber_structs); 	
        free(tids);
    }    
          

    return 0;
}


