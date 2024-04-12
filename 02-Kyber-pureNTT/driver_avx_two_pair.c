#include <math.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include "kyber/avx2/poly.h"
#include "../util/freq-utils.h"
#include "../util/util.h"
#include "kyber/ref/randombytes.h"
volatile static int attacker_core_ID;

#define TIME_BETWEEN_MEASUREMENTS 1000000L // 1 millisecond

#define STACK_SIZE 8192*3

struct args_t {
	poly test_poly;
};

static int iters;
static __attribute__((noinline)) void victim(void *varg)
{

	struct args_t *arg = varg;
	// due to avx2 relies a align in vmovdqa in assemble, hence we move it to a temp which could be runned in multi-thread.
	poly temp;
    memcpy(&temp, &arg->test_poly, sizeof(temp));

	while(1)
	{
		poly_invntt_tomont(&temp);
		poly_ntt(&temp);
		poly_reduce(&temp);
	}

	// printf("before invntt:\n");
	// for(int i =0;i<256;i++)
	// {
	// 	// printf("%d ",arg->test_poly.coeffs[i]);
	// 	printf("%d ",temp.coeffs[i]);
	// }

	// // poly_invntt_tomont(&arg->test_poly);
	// poly_invntt_tomont(&temp);
	// printf("invntt:\n");
	// for(int i =0;i<256;i++)
	// {
	// 	// printf("%d ",arg->test_poly.coeffs[i]);
	// 	printf("%d ",temp.coeffs[i]);
	// }
	// printf("\n");
	// // poly_ntt(&arg->test_poly);
	// poly_ntt(&temp);
	// // poly_reduce(&arg->test_poly);
	// poly_reduce(&temp);
	// printf("ntt:\n");
	// for(int i =0;i<256;i++)
	// {
	// 	// printf("%d ",arg->test_poly.coeffs[i]);
	// 	printf("%d ",temp.coeffs[i]);
	// }
	// printf("\n");


}

// Collects traces
static __attribute__((noinline)) int monitor(int iszero)
{
	static int rept_index = 0;

	// Pin monitor to a single CPU
	pin_cpu(attacker_core_ID);
	
	// Set filename
	// The format is, e.g., ./out/freq_02_2330.out
	// where 02 is the selector and 2330 is an index to prevent overwriting files
	char output_filename[64];
	sprintf(output_filename, "./out/freq_%02d_%06d.out", iszero, rept_index);
	rept_index += 1;

	// Prepare output file
	FILE *output_file = fopen((char *)output_filename, "w");
	if (output_file == NULL) {
		perror("output file");
	}

	// Prepare
	struct freq_sample_t freq_sample, prev_freq_sample = frequency_msr_raw(attacker_core_ID);

	// Collect measurements
	for (uint64_t i = 0; i < iters; i++) {

		// Wait before next measurement
		nanosleep((const struct timespec[]){{0, TIME_BETWEEN_MEASUREMENTS}}, NULL);

		// Collect measurement
		freq_sample = frequency_msr_raw(attacker_core_ID);

		// Store measurement
		uint64_t aperf_delta = freq_sample.aperf - prev_freq_sample.aperf;
		uint64_t mperf_delta = freq_sample.mperf - prev_freq_sample.mperf;
		uint32_t khz = (maximum_frequency * aperf_delta) / mperf_delta;
		fprintf(output_file, "%" PRIu32 "\n", khz);

		// Save current
		prev_freq_sample = freq_sample;
	}

	// Clean up
	fclose(output_file);
	return 0;
}

int main(int argc, char *argv[])
{
	// Check arguments
	if (argc != 4) {
		fprintf(stderr, "Wrong Input! Enter: %s <ntasks> <samples> <outer>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Read in args
	int ntasks;
	int outer;
	sscanf(argv[1], "%d", &ntasks);
	if (ntasks < 0) {
		fprintf(stderr, "ntasks cannot be negative!\n");
		exit(1);
	}
	sscanf(argv[2], "%" PRIu64, &(iters));
	sscanf(argv[3], "%d", &outer);
	if (outer < 0) {
		fprintf(stderr, "outer cannot be negative!\n");
		exit(1);
	}

	// Set the scheduling priority to high to avoid interruptions
	// (lower priorities cause more favorable scheduling, and -20 is the max)
	setpriority(PRIO_PROCESS, 0, -20);

	// Prepare up monitor/attacker
	attacker_core_ID = 0;
	set_frequency_units(attacker_core_ID);
	frequency_msr_raw(attacker_core_ID);

	// Allocate memory for the threads
	char *tstacks = mmap(NULL, (ntasks + 1) * STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	//1. non-zero tests.
	struct args_t arg;
	for (int i = 0; i < outer;i++)
	{

		int tids[ntasks];
		uint rand_pair;
		do{
			randombytes(&rand_pair, sizeof(rand_pair));
			rand_pair %=128;
		}while(rand_pair==0);
		
		rand_pair = rand_pair*2;
		int rand_index = rand_pair/32 *32+ rand_pair%32/2;
		

			// 2. one-zero poly invntt

		poly C00C;

		for(int i = 0 ; i < 256; ++i)
		{
			C00C.coeffs[i] = 0;
		}
		// C_10, .... 0C_2,...... ; the next C0 is on index==rand_pair;
		// rand_pair holds the same.
	

		randombytes(&C00C.coeffs[0],sizeof(C00C.coeffs[0]));
		randombytes(&C00C.coeffs[rand_index+16],sizeof(C00C.coeffs[rand_index+16]));	
		poly_reduce(&C00C);

		memcpy(&arg.test_poly, &C00C,sizeof(C00C));

		#if (SLEEP == 1)
			// Cool down
			sleep(30);
		#endif

			// Start victim threads
			// int tids[ntasks];
		for (int tnum = 0; tnum < ntasks; tnum++) {
			tids[tnum] = clone(&victim, tstacks + (ntasks - tnum) * STACK_SIZE, CLONE_VM | SIGCHLD, &arg);
		}

		// Start the monitor thread
		clone(&monitor, tstacks + (ntasks + 1) * STACK_SIZE, CLONE_VM | SIGCHLD, 3);

		// Join monitor thread
		wait(NULL);

		// Kill victim threads
		for (int tnum = 0; tnum < ntasks; tnum++) {
			syscall(SYS_tgkill, tids[tnum], tids[tnum], SIGTERM);

			// Need to join o/w the threads remain as zombies
			// https://askubuntu.com/a/427222/1552488
			wait(NULL);
		}	


		poly C0C0;

		for(int i = 0 ; i < 256; ++i)
		{
			C0C0.coeffs[i] = 0;
		}


		//avx index transformation
		// C0, .... C0,...... ; the next C0 is on index==rand_pair;
		randombytes(&C0C0.coeffs[0],sizeof(C0C0.coeffs[0]));
		randombytes(&C0C0.coeffs[rand_index],sizeof(C0C0.coeffs[1]));		
		poly_reduce(&C0C0);

	#if (SLEEP == 1)
			// Cool down
			sleep(30);
	#endif

		memcpy(&arg.test_poly, &C0C0,sizeof(C0C0));

		// Start victim threads

		for (int tnum = 0; tnum < ntasks; tnum++) {
			tids[tnum] = clone(&victim, tstacks + (ntasks - tnum) * STACK_SIZE, CLONE_VM | SIGCHLD, &arg);
		}

		// Start the monitor thread
		clone(&monitor, tstacks + (ntasks + 1) * STACK_SIZE, CLONE_VM | SIGCHLD, 4);

		// Join monitor thread
		wait(NULL);

		// Kill victim threads
		for (int tnum = 0; tnum < ntasks; tnum++) {
			syscall(SYS_tgkill, tids[tnum], tids[tnum], SIGTERM);

			// Need to join o/w the threads remain as zombies
			// https://askubuntu.com/a/427222/1552488
			wait(NULL);
		}

	}
}
