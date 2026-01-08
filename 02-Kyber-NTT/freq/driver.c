#include <math.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <string.h>
#include <getopt.h>

/* Conditional includes based on implementation */
#ifdef USE_AVX2
#include "../kyber/avx2/poly.h"
#else
#include "../kyber/ref/poly.h"
#endif

#include "../util/freq-utils.h"
#include "../util/util.h"
#include "../util/rapl-utils.h"
#include "../kyber/ref/randombytes.h"

/* Configuration */
#define TIME_BETWEEN_MEASUREMENTS 1000000L  // 1 millisecond
#define STACK_SIZE (8192 * 3)
#define VICTIM_WARMUP_TIME 10  // seconds

/* Test modes */
typedef enum {
	MODE_SINGLE_CIPHER,
	MODE_DUAL_CIPHER
} test_mode_t;

/* Global state */
static volatile int monitor_core_id;
static uint64_t measurement_samples;

/* Thread arguments */
typedef struct {
	poly test_poly;
} victim_args_t;

/* Victim thread: continuously performs NTT operations */
static __attribute__((noinline)) int victim_thread(void *arg)
{
	victim_args_t *victim_args = (victim_args_t *)arg;

#ifdef USE_AVX2
	/* AVX2 implementation requires a local copy due to alignment requirements */
	poly temp;
	memcpy(&temp, &victim_args->test_poly, sizeof(temp));

	while (1) {
		poly_invntt_tomont(&temp);
		poly_ntt(&temp);
		poly_reduce(&temp);
	}
#else
	/* Reference implementation can use the shared poly directly */
	while (1) {
		poly_invntt_tomont(&victim_args->test_poly);
		poly_ntt(&victim_args->test_poly);
	}
#endif

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
	snprintf(filename, sizeof(filename), "./data/tmp/freq_%02d_%06d.out", test_id, trace_index++);

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

/* Initialize polynomial for single cipher mode */
static void init_single_cipher_poly(poly *p, int test_num)
{
	// Clear all coefficients
	memset(p->coeffs, 0, sizeof(p->coeffs));

#ifdef USE_AVX2
	// AVX2: due to register layout, even-indexed coeffs are stored differently
	if (test_num == 0) {
		// Test 0: Two non-zero coefficients at positions 0 and 16
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
		randombytes((uint8_t *)&p->coeffs[16], sizeof(p->coeffs[16]));
	} else {
		// Test 1: Single non-zero coefficient at position 0
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
	}
#else
	// Reference: straightforward indexing
	if (test_num == 0) {
		// Test 0: Two adjacent non-zero coefficients
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
		randombytes((uint8_t *)&p->coeffs[1], sizeof(p->coeffs[1]));
	} else {
		// Test 1: Single non-zero coefficient
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
	}
#endif

	poly_reduce(p);
}

/* Initialize polynomial for dual cipher mode */
static void init_dual_cipher_poly(poly *p, int test_num)
{
	// Clear all coefficients
	memset(p->coeffs, 0, sizeof(p->coeffs));

	// Generate random spacing between coefficients
	uint rand_pair;
	do {
		randombytes((uint8_t *)&rand_pair, sizeof(rand_pair));
		rand_pair %= 128;
	} while (rand_pair == 0);

#ifdef USE_AVX2
	// AVX2: special index transformation for register layout
	rand_pair = rand_pair * 2;
	int rand_index = rand_pair / 32 * 32 + rand_pair % 32 / 2;

	if (test_num == 3) {
		// Test 3: C00C pattern - coeffs at 0 and rand_index+16
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
		randombytes((uint8_t *)&p->coeffs[rand_index + 16], sizeof(p->coeffs[rand_index + 16]));
	} else {
		// Test 4: C0C0 pattern - coeffs at 0 and rand_index
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
		randombytes((uint8_t *)&p->coeffs[rand_index], sizeof(p->coeffs[rand_index]));
	}
#else
	// Reference: straightforward indexing with random spacing
	if (test_num == 3) {
		// Test 3: C00C pattern - coeffs at 0 and 2*rand_pair+1
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
		randombytes((uint8_t *)&p->coeffs[2 * rand_pair + 1], sizeof(p->coeffs[2 * rand_pair + 1]));
	} else {
		// Test 4: C0C0 pattern - coeffs at 0 and 2*rand_pair
		randombytes((uint8_t *)&p->coeffs[0], sizeof(p->coeffs[0]));
		randombytes((uint8_t *)&p->coeffs[2 * rand_pair], sizeof(p->coeffs[2 * rand_pair]));
	}
#endif

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

/* Print usage information */
static void print_usage(const char *prog_name)
{
	fprintf(stderr, "Usage: %s [OPTIONS] <num_threads> <samples> <iterations>\n\n", prog_name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  --mode <single|dual>  Test mode (default: single)\n");
	fprintf(stderr, "                        single: Single cipher test\n");
	fprintf(stderr, "                        dual:   Dual cipher test\n");
	fprintf(stderr, "  -h, --help            Show this help message\n\n");
	fprintf(stderr, "Arguments:\n");
	fprintf(stderr, "  num_threads           Number of victim threads\n");
	fprintf(stderr, "  samples               Number of measurements to collect\n");
	fprintf(stderr, "  iterations            Number of test iterations\n");
#ifdef USE_AVX2
	fprintf(stderr, "\nImplementation: AVX2\n");
#else
	fprintf(stderr, "\nImplementation: Reference\n");
#endif
}

int main(int argc, char *argv[])
{
	test_mode_t mode = MODE_SINGLE_CIPHER;  // Default mode

	// Parse options
	static struct option long_options[] = {
		{"mode", required_argument, 0, 'm'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}
	};

	int opt;
	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "h", long_options, &option_index)) != -1) {
		switch (opt) {
		case 'm':
			if (strcmp(optarg, "single") == 0) {
				mode = MODE_SINGLE_CIPHER;
			} else if (strcmp(optarg, "dual") == 0) {
				mode = MODE_DUAL_CIPHER;
			} else {
				fprintf(stderr, "Error: Invalid mode '%s'\n", optarg);
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			break;
		case 'h':
			print_usage(argv[0]);
			exit(EXIT_SUCCESS);
		default:
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	// Parse positional arguments
	if (argc - optind != 3) {
		fprintf(stderr, "Error: Missing required arguments\n\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	int num_threads = atoi(argv[optind]);
	measurement_samples = strtoull(argv[optind + 1], NULL, 10);
	int num_iterations = atoi(argv[optind + 2]);

	if (num_threads <= 0 || num_iterations <= 0) {
		fprintf(stderr, "Error: num_threads and iterations must be positive\n");
		exit(EXIT_FAILURE);
	}

	// Print configuration
#ifdef USE_AVX2
	printf("Implementation: AVX2\n");
#else
	printf("Implementation: Reference\n");
#endif
	printf("Mode: %s cipher\n", mode == MODE_SINGLE_CIPHER ? "Single" : "Dual");
	printf("Threads: %d, Samples: %lu, Iterations: %d\n\n",
	       num_threads, measurement_samples, num_iterations);

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

	// Run tests based on mode
	for (int iteration = 0; iteration < num_iterations; iteration++) {
		poly test_poly;

		if (mode == MODE_SINGLE_CIPHER) {
			// Single cipher mode: tests 0 and 1
			init_single_cipher_poly(&test_poly, 0);
			run_test(0, &test_poly, num_threads, thread_stacks);

			init_single_cipher_poly(&test_poly, 1);
			run_test(1, &test_poly, num_threads, thread_stacks);
		} else {
			// Dual cipher mode: tests 3 and 4
			init_dual_cipher_poly(&test_poly, 3);
			run_test(3, &test_poly, num_threads, thread_stacks);

			init_dual_cipher_poly(&test_poly, 4);
			run_test(4, &test_poly, num_threads, thread_stacks);
		}
	}

	// Cleanup
	munmap(thread_stacks, total_stack_size);

	printf("Tests completed successfully.\n");
	return 0;
}
