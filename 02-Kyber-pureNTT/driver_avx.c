#include <math.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include "kyber/avx2/poly.h"
#include "../util/freq-utils.h"
#include "../util/util.h"
#include "../util/rapl-utils.h"
#include "kyber/ref/randombytes.h"

/* Configuration */
#define TIME_BETWEEN_MEASUREMENTS 1000000L  // 1 millisecond
#define STACK_SIZE (8192 * 3)
#define VICTIM_WARMUP_TIME 10  // seconds

/* Global state */
static volatile int monitor_core_id;
static uint64_t measurement_samples;

/* Thread arguments */
typedef struct {
	poly test_poly;
} victim_args_t;

/* Victim thread: continuously performs NTT operations
 * Note: AVX2 implementation requires a local copy due to alignment requirements
 */
static __attribute__((noinline)) int victim_thread(void *arg)
{
	victim_args_t *victim_args = (victim_args_t *)arg;
	poly temp;
	memcpy(&temp, &victim_args->test_poly, sizeof(temp));

	while (1) {
		poly_invntt_tomont(&temp);
		poly_ntt(&temp);
		poly_reduce(&temp);
	}

	return 0;
}

/* Monitor thread: collects frequency and energy measurements */
static __attribute__((noinline)) int monitor_thread(void *arg)
{
	static int trace_index = 0;
	int test_id = (int)(long)arg;

	// Pin to monitoring core
	pin_cpu(monitor_core_id);

	// Open output file
	char filename[64];
	snprintf(filename, sizeof(filename), "./out/freq_%02d_%06d.out", test_id, trace_index++);

	FILE *output = fopen(filename, "w");
	if (!output) {
		perror("Failed to open output file");
		return 1;
	}

	// Initialize measurements
	double prev_energy = rapl_msr(monitor_core_id, PP0_ENERGY);
	struct freq_sample_t prev_freq = frequency_msr_raw(monitor_core_id);

	// Collect measurements
	for (uint64_t i = 0; i < measurement_samples; i++) {
		// Wait before next sample
		nanosleep((const struct timespec[]){{0, TIME_BETWEEN_MEASUREMENTS}}, NULL);

		// Read current values
		double energy = rapl_msr(monitor_core_id, PP0_ENERGY);
		struct freq_sample_t freq = frequency_msr_raw(monitor_core_id);

		// Calculate frequency
		uint64_t aperf_delta = freq.aperf - prev_freq.aperf;
		uint64_t mperf_delta = freq.mperf - prev_freq.mperf;
		uint32_t khz = (maximum_frequency * aperf_delta) / mperf_delta;

		// Write to file
		fprintf(output, "%.15f %" PRIu32 "\n", energy - prev_energy, khz);

		// Update previous values
		prev_energy = energy;
		prev_freq = freq;
	}

	fclose(output);
	return 0;
}

/* Initialize a polynomial with given pattern
 * For AVX2: coeffs[0] and coeffs[16] due to AVX2 register layout
 * AVX2 stores even-indexed coeffs [r[0], r[2], ..., r[30]] in first 16 elements
 */
static void init_poly_pattern(poly *p, int num_nonzero_coeffs)
{
	// Clear all coefficients
	memset(p->coeffs, 0, sizeof(p->coeffs));

	// Set random non-zero coefficients
	if (num_nonzero_coeffs >= 1) {
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
	}
	if (num_nonzero_coeffs >= 2) {
		// For AVX2, the next odd-indexed coeff is at position 16
		randombytes((uint8_t *)&p->coeffs[16], sizeof(p->coeffs[16]));
	}

	poly_reduce(p);
}

/* Start victim threads */
static void start_victim_threads(int *tids, int num_threads, char *stacks,
                                  int stack_size, victim_args_t *args)
{
	for (int i = 0; i < num_threads; i++) {
		tids[i] = clone(&victim_thread,
		                stacks + (num_threads - i) * stack_size,
		                CLONE_VM | SIGCHLD,
		                args);
	}
}

/* Stop and wait for victim threads */
static void stop_victim_threads(int *tids, int num_threads)
{
	for (int i = 0; i < num_threads; i++) {
		syscall(SYS_tgkill, tids[i], tids[i], SIGTERM);
		wait(NULL);  // Reap zombie
	}
}

/* Run a single test iteration */
static void run_test(int test_id, poly *test_poly, int num_threads, char *stacks)
{
	victim_args_t args;
	memcpy(&args.test_poly, test_poly, sizeof(poly));

	// Start victim threads
	int tids[num_threads];
	start_victim_threads(tids, num_threads, stacks, STACK_SIZE, &args);

	// Wait for victims to warm up
	sleep(VICTIM_WARMUP_TIME);

	// Start monitor thread
	clone(&monitor_thread,
	      stacks + (num_threads + 1) * STACK_SIZE,
	      CLONE_VM | SIGCHLD,
	      (void *)(long)test_id);

	// Wait for monitor to finish
	wait(NULL);

	// Stop victim threads
	stop_victim_threads(tids, num_threads);
}

int main(int argc, char *argv[])
{
	// Parse arguments
	if (argc != 4) {
		fprintf(stderr, "Usage: %s <num_threads> <samples> <iterations>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int num_threads = atoi(argv[1]);
	measurement_samples = strtoull(argv[2], NULL, 10);
	int num_iterations = atoi(argv[3]);

	if (num_threads <= 0 || num_iterations <= 0) {
		fprintf(stderr, "Error: num_threads and iterations must be positive\n");
		exit(EXIT_FAILURE);
	}

	// Set high priority for better timing accuracy
	setpriority(PRIO_PROCESS, 0, -20);

	// Initialize monitoring core
	monitor_core_id = 0;
	set_frequency_units(monitor_core_id);
	frequency_msr_raw(monitor_core_id);
	set_rapl_units(monitor_core_id);
	rapl_msr(monitor_core_id, PP0_ENERGY);

	// Allocate thread stacks
	size_t total_stack_size = (num_threads + 1) * STACK_SIZE;
	char *thread_stacks = mmap(NULL, total_stack_size,
	                            PROT_READ | PROT_WRITE,
	                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (thread_stacks == MAP_FAILED) {
		perror("Failed to allocate thread stacks");
		exit(EXIT_FAILURE);
	}

	// Run tests
	for (int iteration = 0; iteration < num_iterations; iteration++) {
		poly test_poly;

		// Test 1: Two non-zero coefficients (AVX2 layout: positions 0 and 16)
		init_poly_pattern(&test_poly, 2);
		run_test(0, &test_poly, num_threads, thread_stacks);

		// Test 2: Single non-zero coefficient
		init_poly_pattern(&test_poly, 1);
		run_test(1, &test_poly, num_threads, thread_stacks);
	}

	// Cleanup
	munmap(thread_stacks, total_stack_size);

	return 0;
}
