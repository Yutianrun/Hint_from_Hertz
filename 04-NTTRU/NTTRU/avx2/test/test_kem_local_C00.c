#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cpucycles.h"
#include "speed.h"
#include "../randombytes.h"
#include "../params.h"
#include "../crypto_stream.h"
#include "../ntru.h"
#include "../kem.h"
#include "../poly.h"
// #include "../ntt.h"
#include <sys/time.h>
#include <pthread.h>

// poly fhat1=  {{3720,209,4954,7202,7214,1239,7058,1779,7437,5554,6266,309,4950,3540,5819,487,4099,3382,2350,5337,2358,2118,780,3525,3230,6152,5901,5674,2787,2456,5420,3934,3896,3835,4662,6122,4286,744,351,6427,2566,767,4506,6244,4688,1065,5188,1684,5246,5132,1028,3547,1933,3900,7294,4963,5617,7393,611,3984,4112,2075,3734,4835,4291,4373,6278,4443,6453,4820,4352,6609,7358,1337,4524,4744,5019,3574,1530,672,5644,6362,3393,4066,7630,1895,991,2305,1275,4762,5851,2589,5374,6546,5539,432,2264,1922,2855,5270,5416,3517,6607,151,7650,2423,4956,6491,3880,7086,1948,74,2949,1533,2359,4558,7080,4457,3845,3878,609,2903,2512,5528,6211,6956,138,4683,2910,6707,4266,611,5783,2103,5241,2771,4101,992,7276,5619,3112,6857,4800,4968,4798,6115,3838,3227,3410,5959,5196,2297,6977,69,6751,5217,1265,7656,5921,7601,3685,4775,4043,4109,6697,1805,1349,5037,2217,3744,598,6992,2768,5851,597,3776,2438,1357,2150,6845,5657,6489,3054,6016,6785,2112,2414,553,490,1630,7374,1125,2816,3969,2884,5765,4902,5095,1271,1642,2227,5949,3705,4102,3546,5715,524,2009,6665,5732,93,7105,938,7664,4483,1623,3222,4116,4673,4991,6454,2879,4055,1866,2900,4245,2840,848,2972,7325,2459,5435,881,3905,2183,6593,5148,5713,5463,5005,3741,2988,509,5357,2312,4077,1304,4164,3285,4739,6185,7191,1031,5063,6791,6058,2005,1636,7314,2895,5192,1937,7446,5052,3190,3946,4823,2613,5851,4285,6934,2564,6524,5172,6980,5213,6087,1871,2461,3998,3795,4655,4819,7233,5147,433,3499,375,3716,3939,6713,3548,4976,4430,4970,6006,1545,4711,5674,2221,3609,5844,5238,2674,4651,3986,5683,4617,1732,6891,2859,263,6997,2882,2744,250,3005,3705,4527,4657,4380,354,1148,7560,7420,6431,6030,267,3467,5702,121,1983,4585,2384,1356,2281,4961,1091,6739,6371,5268,2855,42,4389,3912,6762,1000,3096,4837,4494,2178,2334,1106,3477,7562,650,1878,2768,2264,2814,1895,6527,467,6349,3615,7420,2124,7566,4209,6874,3639,301,2804,3857,376,2685,6466,5860,2901,5266,1451,6655,5429,6456,5536,6114,5766,563,1784,7551,1537,2931,7438,4986,336,435,6770,6431,7528,784,808,1536,832,6044,1965,7140,7143,4936,5285,160,7542,7430,4094,371,7572,7492,6436,7573,2098,941,2251,3928,3761,628,6389,4095,6525,5980,5054,6557,6736,3345,4663,4882,1817,4309,4561,191,1943,5555,1146,2577,1878,6622,5779,5254,3630,2009,4205,105,1340,6499,4308,2415,1577,1896,2548,7679,3277,6843,1066,6301,4654,591,5719,6542,5672,1246,2342,1143,352,6143,6706,3469,4996,7335,2523,7296,1769,2524,3962,5183,555,3114,4903,3880,7179,4755,687,6873,6034,5742,1277,2273,1101,338,5076,4154,243,3502,7042,2137,5572,7036,3496,5588,4561,3543,5614,5101,5476,1894,6315,1179,5820,3465,4970,914,5493,4969,1751,6892,4100,7123,1414,6727,3322,7381,2939,3154,3884,6400,6792,2011,3379,511,1491,269,5254,3035,1604,64,3343,5601,2514,2035,5493,6819,6948,7010,5249,7036,2579,2860,2608,898,465,7223,7411,859,5674,278,2567,4606,6536,1126,1756,1097,4276,6900,1215,5750,6085,4680,857,7542,3505,6306,3703,5427,6988,5369,610,3597,1237,1173,2549,3742,6367,4357,128,3035,7006,1824,4100,7396,1897,3007,5458,5893,4458,7300,7564,4106,7554,2406,6122,6061,6722,3288,6132,4533,7324,3102,134,2744,5002,6146,999,4386,1485,7415,3279,1799,5215,6035,7339,6046,4459,3462,4420,4993,6939,7273,3446,6156,3295,2102,1877,5922,1735,7187,2548,182,6399,732,294,6005,5317,7566,3730,6155,7528,3560,4299,3984,6244,386,1564,5690,104,6466,437,1811,2159,2693,956,1811,5557,814,947,4109,2109,1034,5983,2949,7648,2522,5468,7358,2812,4404,1099,4191,3186,2119,6483,4758,1232,3316,4605,5401,1299,4035,2867,3160,2284,1480,3989,4764,5515,6589,7276,4637,2890,2728,6387,803,3298,5683,1468,3712,7190,3276,897,7557,6545,6975,6659,2456,6970,1502,6981,5099,912,5856,1087,5659,6101,3618,2735,5933,3776,5175,1301,2971,6744,5272,1592,2579,3212,2789,3043,2962,2937,4909,7499,4459,5242,5720,7237,6339,1305,3769,5298,6357,4979,6456,2760,2983,1107,3382,4584,2175,1386,1280,839,2030}};

typedef struct {
    unsigned char k[SHAREDKEYBYTES];
    unsigned char c[CIPHERTEXTBYTES];
    unsigned char sk[SECRETKEYBYTES];

} nttru_dec_struct;

void * nttru_thread(void *args)
{
    nttru_dec_struct *actual_args = args;
    unsigned char k[SHAREDKEYBYTES];
    unsigned char c[CIPHERTEXTBYTES];
    unsigned char sk[SECRETKEYBYTES];
    // memcpy(k, &actual_args->k, sizeof(k));
    memset(k,0,sizeof(k));
    memcpy(c, &actual_args->c, sizeof(c));
    memcpy(sk, &actual_args->sk, sizeof(sk));
    while(1)
    // ntru_decrypt(&actual_args->m, &actual_args->chat, &actual_args->fhat);
    crypto_kem_dec(k, c, sk);
    return 0;
}


int main(int argc, char *argv[]) {


    int guess_z1 = atoi(argv[1]);
    int guess_z2 = atoi(argv[2]);
    int target_tripple_index = atoi(argv[3]);
    int number_thread = atoi(argv[4]);
    int iteration = atoi(argv[5]);
    
    // int key_index= atoi(argv[5]);

    // printf("hit_difference %d\n", hit_difference); 
    // printf("target_tripple_index %d\n", target_tripple_index);
    // printf("number_thread %d\n", number_thread); 
    // printf("iteration %d\n", iteration); 
    // printf("key_index %d\n", key_index); 
    // init_ntt();

    // memcpy(&fhat, &fhat1, sizeof(fhat));
    poly fhat,hhat;
    poly chat;
    unsigned char c[CIPHERTEXTBYTES];
    unsigned char pk[PUBLICKEYBYTES], sk[SECRETKEYBYTES];
    unsigned char coins[N]={57,82,11,16,239,104,27,164,156,148,158,91,108,136,43,250,37,42,142,104,210,104,212,236,225,158,112,95,84,110,17,246,90,190,210,71,240,97,255,133,102,181,30,137,202,29,0,199,166,200,69,168,228,160,159,216,43,146,171,58,50,13,158,113,64,145,183,213,80,131,194,5,140,197,0,212,39,237,25,22,62,12,66,241,135,77,118,234,117,13,113,130,28,36,249,135,193,30,122,166,92,85,169,11,132,65,238,176,16,136,41,60,160,251,136,8,210,158,204,215,110,196,179,10,114,30,188,242,71,62,217,123,102,187,170,220,187,29,252,113,190,208,234,185,6,217,200,190,181,86,182,0,3,162,161,163,181,147,23,85,181,69,216,4,103,172,145,19,79,180,149,174,255,19,93,65,163,4,16,112,57,129,24,122,84,123,116,157,212,30,69,140,33,85,45,73,56,222,4,190,46,184,194,141,75,188,57,12,251,218,48,238,67,46,10,117,131,190,159,220,175,14,107,234,174,196,206,140,17,58,67,168,118,60,180,129,72,129,131,112,142,29,65,35,190,203,87,125,198,35,51,127,214,191,193,2,147,2,243,88,26,247,158,30,7,206,85,220,124,53,1,79,116,51,53,222,111,242,88,33,168,123,116,205,221,98,151,11,43,144,58,185,143,76,57,248,134,18,95,133,28,3,165,184,29,104,12,127,209,43,58,20,158,149,158,236,143,150,139,245,178,161,86,65,68,245,237,23,205,59,31,239,21,194,198,135,103,40,148,183,114,167,122,169,187,137,62,248,149,211,197,186,31,110,45,5,192,93,165,232,119,180,226,54,168,30,108,12,250,139,237,21,191,53,223,219,119,192,27,222,238,106,142,22,219,246,52,29,189,156,211,253,200,162,185,10,125,236,113,32,104,88,189,113,104,19,239,94,155,205,119,172,83,60,81,192,157,147,223,189,47,253,214,71,62,130,103,101,56,11,87,234,29,64,190,115,228,114,134,155,1,131,6,18,81,157,105,171,204,87,58,111,83,254,35,248,250,107,145,146,76,191,34,215,36,132,220,45,89,175,187,60,64,12,72,135,221,182,2,79,40,170,30,255,144,1,142,111,57,145,220,199,201,58,96,134,138,123,189,211,159,248,81,172,86,161,187,124,74,54,46,246,201,211,176,226,206,171,19,191,10,153,119,182,195,245,55,26,206,108,195,0,23,240,235,115,177,90,141,5,90,102,109,150,164,196,106,150,119,222,244,195,103,146,48,84,213,88,193,219,116,188,241,142,210,63,210,169,82,124,203,163,197,246,223,93,44,142,194,86,63,70,23,247,39,218,12,123,179,52,120,252,29,178,121,145,20,52,133,206,120,222,247,247,172,135,204,226,85,23,96,142,240,91,70,97,232,54,115,99,23,29,219,93,75,229,93,56,137,156,170,241,78,227,93,229,110,110,217,186,174,240,115,193,4,188,192,74,108,224,245,38,44,125,194,212,237,171,119,159,83,86,225,122,241,37,17,54,30,72,193,158,210,67,108,153,56,90,47,183,242,144,171,169,91,108,148,218,186,248,62,77,164,68,201,11,152,108,12,224,254,236,17,18,2,135,204,161,76,25,183,139,9,153,44,73,64,218,44,162,214,230,17,184,86,233,86,252,248,251,216,186,48,203,221,18,231,142,250,255,20,39,56,99,197,169,3,64,41,130,56,69,107,241,31,198,77,134,216,76,77,39,123,188,237,49,212,198,};
    ntru_keygen(&hhat,&fhat,coins);
    // for (size_t i = 0; i < 768; i++)
    // {
    //     /* code */
    //     printf("%d,",fhat.coeffs[i]);
    // }
    // printf("\n");
    
    // getchar();
    poly_pack_uniform(pk, &hhat);
    poly_pack_uniform(sk, &fhat);
    for(int i = 0; i < PUBLICKEYBYTES; ++i)
        sk[i + POLY_PACKED_UNIFORM_BYTES] = pk[i];
    


    crypto_kem_malice_enc1(c, guess_z1, guess_z2,target_tripple_index);
    nttru_dec_struct *nttru_structs = (nttru_dec_struct *)malloc(number_thread * sizeof(nttru_dec_struct));
    for (int j = 0; j < number_thread; j++)
    {
        memcpy(&nttru_structs[j].c, c, sizeof(c));
        memcpy(&nttru_structs[j].sk, sk, sizeof(sk));
    }
    pthread_t *tids = (pthread_t *)malloc(number_thread * sizeof(pthread_t));

    for(int i =0; i<iteration; i++){
        
            
        // struct timespec tstart={0,0}, tend={0,0};
        // clock_gettime(CLOCK_MONOTONIC, &tstart);
        
        for(int j = 0; j < number_thread; j++){
            pthread_create(&tids[j], NULL, nttru_thread, &nttru_structs[j]);	
        }
        
       
        for (int j = 0; j < number_thread; j++){
            pthread_join(tids[j], NULL);
        }
        
        // clock_gettime(CLOCK_MONOTONIC, &tend);
        // printf("nttru took about %.5f seconds\n",
        //         ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - 
        //         ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec));
            
        
    }    
    free(nttru_structs); 	
    free(tids);
    return 0;
}
