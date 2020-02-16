#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <immintrin.h>
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <sys/types.h>

static char stop = 0;
static unsigned long *results;
static unsigned long *stages;
static int n_stages;
static char avxfreq = 0;

static pthread_barrier_t barrier;

static unsigned long execute_scalar() {
	unsigned long i;

	if(avxfreq)
		syscall(430, 0);

	for(i = 0; i < ULONG_MAX; i++) {
		if(stop)
			break;
	}

	return i;
}

static unsigned long execute_avx_light() {
	unsigned long i;

	if(avxfreq)
		syscall(430, 1);

	for(i = 0; i < ULONG_MAX; i++) {
		if(stop)
			break;

		asm volatile(
			"vfmaddsub132pd %zmm0, %zmm1, %zmm2\n"
		);
	}

	return i;
}

static unsigned long execute_avx_heavy() {
	unsigned long i;

	if(avxfreq)
		syscall(430, 2);

	for(i = 0; i < ULONG_MAX; i++) {
		if(stop)
			break;

		asm volatile(
			"vfmaddsub132pd %zmm0, %zmm0, %zmm1\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm2\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm3\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm4\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm5\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm6\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm7\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm8\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm9\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm10\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm11\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm12\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm13\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm14\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm15\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm16\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm17\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm18\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm19\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm20\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm21\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm22\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm23\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm24\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm25\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm26\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm27\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm28\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm29\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm30\n"
			"vfmaddsub132pd %zmm0, %zmm0, %zmm31\n"
		);
	}

	return i;
}

static void *run_exec_thread(void *unused) {
	pthread_barrier_wait(&barrier);
	cpu_set_t set;
	struct sched_param sched_param = { .sched_priority = 99 };

	CPU_SET(0, &set);
	sched_setaffinity(0, sizeof(cpu_set_t), &set);
	sched_setscheduler(0, SCHED_RR, &sched_param);

	for(int i = 0; i < n_stages; i++) {
		if(stages[i] == 0)
			continue;

		switch(i % 3) {
			case 0:
				results[i] = execute_avx_heavy();
				break;
			case 1:
				results[i] = execute_avx_light();
				break;
			case 2:
				results[i] = execute_scalar();
				break;
		}

		stop = 0;
	}

	if(avxfreq)
		syscall(430, 0);

	return NULL;
}

void main(int argc, char **argv) {
	pthread_t exec_thread;

	if(getuid() != 0) {
		printf("must run as root\n");
		exit(1);
	}

	pthread_barrier_init(&barrier, NULL, 2);
	pthread_create(&exec_thread, NULL, run_exec_thread, NULL);

	if(syscall(428)) {
		if(!strcmp(argv[1], "1")) {
			avxfreq = 1;
			syscall(429, 0);
		} /*else {
			syscall(429, 1);
		}*/
	}

	argv = &argv[2];
	argc -= 2;

	results = calloc(argc, sizeof(unsigned long));
	stages = calloc(argc, sizeof(unsigned long));

	n_stages = argc;

	for(int i = 0; i < argc; i++) {
		stages[i] = strtoul(argv[i], NULL, 10);
	}

	pthread_barrier_wait(&barrier);

	for(int i = 0; i < n_stages; i++) {
		if(stages[i] == 0)
			continue;

		usleep(stages[i]);
		stop = 1;
	}

	for(int i = 0; i < n_stages; i++) {
		printf("%i=%lu\n", i, results[i]);
	}

	if(avxfreq) {
		syscall(429, 1);
	}

	free(results);
	free(stages);
}
