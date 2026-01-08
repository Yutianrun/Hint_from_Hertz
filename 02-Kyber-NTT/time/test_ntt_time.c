/*
 * Unified NTT timing test for Kyber (AVX2 and REF implementations)
 *
 * Compile-time selection via macros:
 *   -DKYBER_IMPL_AVX2: Use AVX2 implementation
 *   -DKYBER_IMPL_REF:  Use REF implementation
 *
 * Tests two patterns:
 *   Test 0: [C, C, 0, ..., 0] - two random coefficients
 *   Test 1: [C, 0, 0, ..., 0] - one random coefficient
 *
 * Layout Notes:
 *   AVX2: coeffs[0] and coeffs[16] form a coefficient pair (same NTT position)
 *   REF:  coeffs[0] and coeffs[1] are sequential positions
 *
 * Multi-threading:
 *   - Spawns N threads
 *   - EACH thread independently runs workload_iterations times
 *   - Total operations = N threads × workload_iterations
 *   - Measures the total time for all threads to complete
 *
 * Usage: ./test_ntt_time[_ref] <test_num> <workload_iters> <samples> <threads>
 * Example: ./test_ntt_time 0 10000 1000 8
 *   → Runs test_num=0 ([C,C] pattern)
 *   → 8 threads, each runs 10000 iterations
 *   → Outputs 1000 timing measurements
 */

// Select implementation based on compile-time macro
#if defined(KYBER_IMPL_AVX2)
  #include "../kyber/avx2/poly.h"
  #include "../kyber/avx2/randombytes.h"
  #define IMPL_NAME "AVX2"
  #define COEFF_POS_0 0
  #define COEFF_POS_1 16  // AVX2 pair position
#elif defined(KYBER_IMPL_REF)
  #include "../kyber/ref/poly.h"
  #include "../kyber/ref/randombytes.h"
  #define IMPL_NAME "REF"
  #define COEFF_POS_0 0
  #define COEFF_POS_1 1   // Sequential position
#else
  #error "Must define either KYBER_IMPL_AVX2 or KYBER_IMPL_REF"
#endif

#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Thread argument structure
typedef struct {
    poly test_poly;
    int workload_iterations;
} thread_args_t;

/*
 * Initialize test polynomial with specified pattern
 * test_num 0: [C, C, 0, ..., 0] pattern - two random coefficients
 * test_num 1: [C, 0, 0, ..., 0] pattern - one random coefficient
 */
void init_test_poly(poly *p, int test_num) {
    memset(p->coeffs, 0, sizeof(p->coeffs));

    if (test_num == 0) {
        // Test 0: [C, C] - two random coefficients
        randombytes((uint8_t *)&p->coeffs[COEFF_POS_0], sizeof(p->coeffs[0]));
        randombytes((uint8_t *)&p->coeffs[COEFF_POS_1], sizeof(p->coeffs[0]));
    } else {
        // Test 1: [C, 0] - single random coefficient
        randombytes((uint8_t *)&p->coeffs[COEFF_POS_0], sizeof(p->coeffs[0]));
    }

    poly_reduce(p);
}

/*
 * Thread worker function
 * Each thread runs workload_iterations of NTT/invNTT operations
 */
void *ntt_worker(void *args) {
    thread_args_t *actual_args = (thread_args_t *)args;
    poly temp;
    memcpy(&temp, &actual_args->test_poly, sizeof(temp));

    for(int i = 0; i < actual_args->workload_iterations; i++) {
        poly_invntt_tomont(&temp);
        poly_ntt(&temp);
        poly_reduce(&temp);
    }

    return NULL;
}

/*
 * Measure NTT/invNTT time with multiple threads
 * Returns: elapsed time in seconds
 */
double measure_once(poly *test_poly, int workload_iterations, int num_threads) {
    // Allocate thread structures
    pthread_t *tids = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_args_t *thread_args = (thread_args_t *)malloc(num_threads * sizeof(thread_args_t));

    // Initialize thread arguments
    for(int i = 0; i < num_threads; i++) {
        memcpy(&thread_args[i].test_poly, test_poly, sizeof(poly));
        thread_args[i].workload_iterations = workload_iterations;
    }

    // Start timing
    struct timespec tstart, tend;
    clock_gettime(CLOCK_MONOTONIC, &tstart);

    // Create threads
    for(int i = 0; i < num_threads; i++) {
        pthread_create(&tids[i], NULL, ntt_worker, &thread_args[i]);
    }

    // Wait for all threads to complete
    for(int i = 0; i < num_threads; i++) {
        pthread_join(tids[i], NULL);
    }

    // End timing
    clock_gettime(CLOCK_MONOTONIC, &tend);

    // Cleanup
    free(tids);
    free(thread_args);

    return ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
           ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <test_num> <workload_iters> <samples> <threads>\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Parameters:\n");
        fprintf(stderr, "  test_num         : 0 for [C,C] pattern, 1 for [C,0] pattern\n");
        fprintf(stderr, "                     [C,C] = coeffs at positions 0,16 (AVX2 pair)\n");
        fprintf(stderr, "                     [C,0]  = coeff at position 0 only\n");
        fprintf(stderr, "  workload_iters   : number of NTT/invNTT iterations per thread\n");
        fprintf(stderr, "  samples          : number of time measurements to output\n");
        fprintf(stderr, "  threads          : number of parallel threads per measurement\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Example: %s 0 10000 1000 8\n", argv[0]);
        return 1;
    }

    int test_num = atoi(argv[1]);
    int workload_iterations = atoi(argv[2]);
    int samples = atoi(argv[3]);
    int num_threads = atoi(argv[4]);

    if (test_num < 0 || test_num > 1) {
        fprintf(stderr, "Error: test_num must be 0 or 1\n");
        return 1;
    }

    if (workload_iterations <= 0 || samples <= 0 || num_threads <= 0) {
        fprintf(stderr, "Error: all parameters must be positive\n");
        return 1;
    }

    // Print configuration to stderr (so stdout is clean data)
    fprintf(stderr, "# Configuration:\n");
    fprintf(stderr, "# implementation = %s\n", IMPL_NAME);
    fprintf(stderr, "# test_num = %d (%s pattern)\n", test_num, test_num == 0 ? "[C,C]" : "[C,0]");
    if (test_num == 0) {
        fprintf(stderr, "# Pattern: coeffs at positions %d and %d\n", COEFF_POS_0, COEFF_POS_1);
    } else {
        fprintf(stderr, "# Pattern: coeff at position %d only\n", COEFF_POS_0);
    }
    fprintf(stderr, "# workload_iterations = %d\n", workload_iterations);
    fprintf(stderr, "# samples = %d\n", samples);
    fprintf(stderr, "# threads = %d\n", num_threads);
    fprintf(stderr, "#\n");

    // Initialize test polynomial once
    poly test_poly;
    init_test_poly(&test_poly, test_num);

    // Collect samples
    for(int sample = 1; sample <= samples; sample++) {
        double elapsed = measure_once(&test_poly, workload_iterations, num_threads);
        printf("%.9f\n", elapsed);
    }

    return 0;
}
