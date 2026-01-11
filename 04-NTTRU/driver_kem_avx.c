#include <sys/resource.h>

#include "../util/freq-utils.h"
#include "../util/rapl-utils.h"
#include "../util/util.h"

volatile static int attacker_core_ID;

#define TIME_BETWEEN_MEASUREMENTS 1000000L // 1 millisecond

struct args_t {
	uint64_t iters;
	int16_t guess_z;
	int triple_index;
	int number_thread;
};

static void victim(void *command)
{
	system((char *)command);
}

static __attribute__((noinline)) int monitor(void *in)
{
	static int rept_index = 0;

	struct args_t *arg = (struct args_t *)in;

	// Pin monitor to a single CPU
	pin_cpu(attacker_core_ID);

	// e.g. ./data/tmp/avx_0942_000_00008_000001.out
	char output_filename[96];
	sprintf(output_filename, "./data/tmp/avx_%04hd_%03d_%05d_%06d.out",
	        arg->guess_z, arg->triple_index, arg->number_thread, rept_index);
	rept_index += 1;

	FILE *output_file = fopen((char *)output_filename, "w");
	if (output_file == NULL) {
		perror("output file");
	}

	double energy, prev_energy = rapl_msr(attacker_core_ID, PP0_ENERGY);
	struct freq_sample_t freq_sample, prev_freq_sample = frequency_msr_raw(attacker_core_ID);

	for (uint64_t i = 0; i < arg->iters; i++) {
		nanosleep((const struct timespec[]){{0, TIME_BETWEEN_MEASUREMENTS}}, NULL);

		energy = rapl_msr(attacker_core_ID, PP0_ENERGY);
		freq_sample = frequency_msr_raw(attacker_core_ID);

		uint64_t aperf_delta = freq_sample.aperf - prev_freq_sample.aperf;
		uint64_t mperf_delta = freq_sample.mperf - prev_freq_sample.mperf;
		uint32_t khz = (maximum_frequency * aperf_delta) / mperf_delta;
		fprintf(output_file, "%.15f %" PRIu32 "\n", energy - prev_energy, khz);

		prev_energy = energy;
		prev_freq_sample = freq_sample;
	}

	fclose(output_file);
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Wrong Input! Enter: %s <samples> <outer>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	struct args_t arg;
	int outer;
	sscanf(argv[1], "%" PRIu64, &(arg.iters));
	sscanf(argv[2], "%d", &outer);
	if (outer < 0) {
		fprintf(stderr, "outer cannot be negative!\n");
		exit(1);
	}

	FILE *selectors_file = fopen("input.txt", "r");
	if (selectors_file == NULL)
		perror("fopen error");

	int num_selectors = 0;
	int16_t guess_z[8192];
	int triple_index[8192];
	int number_thread[8192];

	size_t len = 0;
	ssize_t read = 0;
	char *line = NULL;

	while ((read = getline(&line, &len, selectors_file)) != -1) {
		if (line[read - 1] == '\n')
			line[--read] = '\0';

		sscanf(line, "%hd %d %d",
		       &(guess_z[num_selectors]),
		       &(triple_index[num_selectors]),
		       &(number_thread[num_selectors]));
		num_selectors += 1;
		if (num_selectors >= 8192) {
			fprintf(stderr, "Too many selectors (max 8192)\n");
			exit(1);
		}
	}

	setpriority(PRIO_PROCESS, 0, -20);

	attacker_core_ID = 0;
	set_frequency_units(attacker_core_ID);
	frequency_msr_raw(attacker_core_ID);
	set_rapl_units(attacker_core_ID);
	rapl_msr(attacker_core_ID, PP0_ENERGY);

	for (int i = 0; i < outer * num_selectors; i++) {
		arg.guess_z = guess_z[i % num_selectors];
		arg.triple_index = triple_index[i % num_selectors];
		arg.number_thread = number_thread[i % num_selectors];

		pthread_t thread1, thread2;

		char command[256];
		sprintf(command, "./bin/test_kem_local %d %d %d",
		        arg.guess_z, arg.triple_index, arg.number_thread);

		pthread_create(&thread1, NULL, (void *)&victim, (void *)command);
		pthread_create(&thread2, NULL, (void *)&monitor, (void *)&arg);
		pthread_join(thread2, NULL);

		system("pkill -f test_kem_local");
		pthread_join(thread1, NULL);
	}
}

