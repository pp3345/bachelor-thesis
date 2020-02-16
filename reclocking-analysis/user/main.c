#define _GNU_SOURCE

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sched.h>
#include <limits.h>
#include <signal.h>
#include "../include/shared.h"

#define PRE_THROTTLE_DISABLE 0
#define PRE_THROTTLE_AVX 1
#define PRE_THROTTLE_SCALAR 2

#define FP_SP 1
#define FP_DP 2

#define MEASURE_DOWNCLOCK 0
#define MEASURE_UPCLOCK 1
#define MEASURE_PRE_THROTTLE_THROUGHPUT 2
#define MEASURE_THROTTLE_THROUGHPUT 3
#define MEASURE_NON_AVX_TIME 4 // TODO name

#define MIN(x,  y) (((x) < (y)) ? (x) : (y))

static void *avx_memory[3] = {NULL, NULL};
static void *infinite_loop_memory = NULL;
static int ra_dev_fd = -1;
static unsigned long interrupt_license = 0;
static unsigned long threads = 0;
static unsigned long pre_throttle = 0;
static unsigned long measure = 0;
static unsigned long vector_size = 0;
static unsigned char fp_precision = 0;

static pthread_t exec_thread[64];
static pthread_t wait_thread[64];

static volatile char alarm_received = 0;
static volatile char output_cpu = 0;

static pthread_barrier_t barrier;
static pthread_barrier_t thread_barriers[64];

static volatile char try_elapsed = 0;
static volatile char interrupts_received = 0;
static volatile char interrupt_received[64] = {0};
static volatile char ignore_results = 0;

static unsigned long avx_instructions = 0;

/*static void alarm_handler(int signum) {
	alarm_received = 1;
}*/

typedef enum {
	LOOP_R12_CMP,
	LOOP_AVX
} avx_code_mode;

static char map_avx_code(int slot, avx_code_mode mode, char *binary, unsigned long offset, unsigned long length) {
	long page_size = sysconf(_SC_PAGE_SIZE);
	static long pages = 4;
	unsigned long memory_size = page_size * pages;

	while(avx_instructions * length > memory_size) {
		if(avx_memory[slot] != NULL) {
			munmap(avx_memory[slot], memory_size + page_size);
			avx_memory[slot] = NULL;
		}

		pages++;
		memory_size = page_size * pages;
	}

	if(avx_memory[slot] == NULL) {
		avx_memory[slot] = mmap(NULL, memory_size + page_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		fprintf(stderr, "AVX memory (slot %i) initialized at %p - %p (page size %i, %i instructions)\n", slot, avx_memory[slot], avx_memory[slot] + memory_size, page_size, avx_instructions);
	}

	void *start = avx_memory[slot];
	static char *code = NULL;
	static int fd = 0;

	if(code == NULL) {
		code = malloc(length);
		fd = open(binary, O_RDONLY);

		if(fd < 0) {
			fprintf(stderr, "unable to open binary: %i\n", fd);
			exit(1);
		}

		lseek(fd, offset, SEEK_SET);

		if(read(fd, code, length) < (long) length) {
			fprintf(stderr, "failed to read AVX code from binary\n");
			exit(1);
		}

		close(fd);
	}

	unsigned long pos;
	for(pos = 0; pos < MIN(avx_instructions == 0 ? ULONG_MAX : avx_instructions * length, memory_size - length); pos += length) {
		memcpy(start + pos, code, length);
	}

	switch(mode) {
		case LOOP_R12_CMP:
			((char*) start)[pos++] = 0x41; // CMP %0x0, (%r12)
			((char*) start)[pos++] = 0x83;
			((char*) start)[pos++] = 0x3c;
			((char*) start)[pos++] = 0x24;
			((char*) start)[pos++] = 0x00;
			((char*) start)[pos++] = 0x74; // JE $-7
			((char*) start)[pos++] = -7;
			((char*) start)[pos++] = 0xC3; // RET
			// we basically return the SETUP ioctl() call here and discard ioctl()'s stack frame and instruction stream - don't try this at home, kids
			break;
		case LOOP_AVX:
			((char*) start)[pos++] = 0xe9; // jump near relative 32bit
			int jmp_offset = (-1) * ((int) (pos + 4));
			memcpy(start + pos, &jmp_offset, sizeof(int));
			break;
		default:
			fprintf(stderr, "invalid AVX mode\n");
			exit(1);
	}
}

static void map_infinite_loop_code(void) {
	infinite_loop_memory = mmap(NULL, 16, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	((char*) infinite_loop_memory)[0] = 0xeb; // jump short relative 8bit
	((char*) infinite_loop_memory)[1] = -2;

	fprintf(stderr, "Infinite loop initialized at %p\n", infinite_loop_memory);
}

static void *run_wait_thread(void *arg) {
	struct interrupt_result res;
	cpu_set_t set;
	long cpu = (long) arg;

	fprintf(stderr, "wait thread %i starting on cpu %li for cpu %li\n", syscall(__NR_gettid), threads + cpu, cpu);

	res.cpu = cpu;

	CPU_SET(threads + cpu, &set);
	if(sched_setaffinity(0, sizeof(cpu_set_t), &set)) {
		fprintf(stderr, "failed to set CPU affinity: %s\n", strerror(errno));
		exit(1);
	}

	pthread_barrier_wait(&barrier);

	if(ioctl(ra_dev_fd, IOCTL_WAIT_FOR_INTERRUPT, &res)) {
		fprintf(stderr, "WAIT_FOR_INTERRUPT ioctl failed\n");
		exit(1);
	}

	// we have theoretical race condition here which is solely prevented by our assumption that it always takes less than 1ms to switch to the next license level (this is a great example for quality software engineering)
	interrupt_received[cpu] = 1;

	if(measure == MEASURE_NON_AVX_TIME) {
		pthread_barrier_wait(&thread_barriers[cpu]);
		if(ignore_results)
			return NULL;
	} else
		pthread_barrier_wait(&barrier);

	/* quality software engineering */
	while(output_cpu != cpu);

	printf("cpu%li: pmc0: %lx ovf=%u\n", cpu, res.pmcs[0], res.perf_global_status.overflow.pmc0);
	printf("cpu%li: pmc1: %lx ovf=%u\n", cpu, res.pmcs[1], res.perf_global_status.overflow.pmc1);
	printf("cpu%li: pmc2: %lx ovf=%u\n", cpu, res.pmcs[2], res.perf_global_status.overflow.pmc2);
	printf("cpu%li: pmc3: %lx ovf=%u\n", cpu, res.pmcs[3], res.perf_global_status.overflow.pmc3);
	printf("cpu%li: pmc4: %lx ovf=%u\n", cpu, res.pmcs[4], res.perf_global_status.overflow.pmc4);
	printf("cpu%li: pmc5: %lx ovf=%u\n", cpu, res.pmcs[5], res.perf_global_status.overflow.pmc5);
	printf("cpu%li: pmc6: %lx ovf=%u\n", cpu, res.pmcs[6], res.perf_global_status.overflow.pmc6);
	printf("cpu%li: pmc7: %lx ovf=%u\n", cpu, res.pmcs[7], res.perf_global_status.overflow.pmc7);
	printf("cpu%li: inst_retired: %lx\n", cpu, res.fixed_counters.instructions_retired);
	printf("cpu%li: cycles: %lx\n", cpu, res.fixed_counters.cpu_cycles_unhalted);
	printf("cpu%li: cycles_tsc: %lx\n", cpu, res.fixed_counters.cpu_cycles_unhalted_tsc);
	printf("cpu%li: exec_depth: %lx\n", cpu, (unsigned long) (res.ip - (measure != MEASURE_UPCLOCK ? (unsigned long) avx_memory : (unsigned long) infinite_loop_memory)));
	printf("cpu%li: tsc_diff: %lx\n", cpu, res.tsc_end - res.tsc_start);
	if(avx_instructions != 0)
		printf("cpu%li: avx_instructions: %lx\n", cpu, avx_instructions);

	//printf("cpu%li: platform_energy: %lx\n", cpu, res.power.platform_energy);
	//printf("cpu%li: package_energy: %lx\n", cpu, res.power.package_energy);
	printf("cpu%li: perf_limit_reasons: %lx\n", cpu, res.power.perf_limit_reasons);
	//printf("cpu%li: ring_perf_limit_reasons: %lx\n", cpu, res.power.ring_perf_limit_reasons);
	//printf("cpu%li: hwp_min: %lx\n", cpu, res.performance.request.s.min);
	//printf("cpu%li: hwp_max: %lx\n", cpu, res.performance.request.s.max);
	//printf("cpu%li: hwp_desired: %lx\n", cpu, res.performance.request.s.desired);
	//printf("cpu%li: hwp_pref: %lx\n", cpu, res.performance.request.s.pref);
	//printf("cpu%li: hwp_activity_window: %lx\n", cpu, res.performance.request.s.activity_window);
	//printf("cpu%li: hwp_package_control: %lx\n", cpu, res.performance.request.s.package_control);
	//printf("cpu%li: productive_cycles: %lx\n", cpu, res.performance.productive_cycles);
	// energy counters only update every 1ms which is generally too long for our measurements
	// HWP stuff isn't really interesting
	// productive cycles could be interesting if we were able to control when to count (i.e., kernel vs. user)
	// perf limit reasons contain turbo transition attenuation

	output_cpu++;

	fprintf(stderr, "wait thread on cpu %li completed\n", threads + cpu);

	return NULL;
}

static void *run_exec_thread(void *arg) {
	struct setup_args args = {0};
	struct sched_param sched_param = { .sched_priority = 99 };
	cpu_set_t set;
	long cpu = (long) arg;
	char pre_throttle_thread = pre_throttle && cpu != 0;

	//fprintf(stderr, "exec thread %i starting on cpu %li\n", syscall(__NR_gettid), cpu);

	CPU_SET(cpu, &set);
	if(sched_setaffinity(0, sizeof(cpu_set_t), &set)) {
		fprintf(stderr, "failed to set CPU affinity: %s\n", strerror(errno));
		exit(1);
	}

	if(sched_setscheduler(0, SCHED_RR, &sched_param)) {
		fprintf(stderr, "failed to set real-time scheduling: %s\n", strerror(errno));
		exit(1);
	}

	switch(measure) {
		case MEASURE_UPCLOCK:
			args.interrupt_action = INTERRUPT_ACTION_GOTO;
			break;
		case MEASURE_THROTTLE_THROUGHPUT:
			args.interrupt_action = INTERRUPT_ACTION_SET_MSRS;
			break;
		case MEASURE_NON_AVX_TIME:
			if(interrupt_license == 2) {
				args.interrupt_action = INTERRUPT_ACTION_GOTO;
				break;
			}
			/* fallthru */
		default:
			args.interrupt_action = INTERRUPT_ACTION_WAKE_WAIT_THREAD;
			break;
	}

	args.ip = (unsigned long) avx_memory[0];
	if(measure == MEASURE_NON_AVX_TIME) {
		args.r12 = (unsigned long) &try_elapsed;

		if(pre_throttle_thread && pre_throttle == PRE_THROTTLE_AVX)
			args.ip = (unsigned long) avx_memory[2];
	}

	args.msrs[0] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_SEL_0,
		.val = ((union pmc_sel) { .s = {
			.event = PM_EVENT_CORE_POWER,
			.umask = PM_EVENT_CORE_POWER_LVL0_TURBO_LICENSE_UMASK,
			.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_EN/* | (measure == MEASURE_THROTTLE_THROUGHPUT ? PMC_SEL_FLAG_OS : 0)*/, // we want to measure throughput while throttled, so kernel time would make our measurement inaccurate - however, this way this isn't suitable to measure total throttle cycles
		}}).u,
	};
	args.msrs[1] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_SEL_1,
		.val = ((union pmc_sel) { .s = {
			.event = PM_EVENT_CORE_POWER,
			.umask = PM_EVENT_CORE_POWER_LVL1_TURBO_LICENSE_UMASK,
			.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_EN | ((interrupt_license == 1 || (interrupt_license == 2 && measure == MEASURE_NON_AVX_TIME)) && !pre_throttle_thread ? PMC_SEL_FLAG_INT : 0) | (measure == MEASURE_UPCLOCK || measure == MEASURE_THROTTLE_THROUGHPUT ? PMC_SEL_FLAG_OS : 0),
		}}).u,
	};
	args.msrs[2] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_SEL_2,
		.val = ((union pmc_sel) { .s = {
			.event = PM_EVENT_CORE_POWER,
			.umask = PM_EVENT_CORE_POWER_LVL2_TURBO_LICENSE_UMASK,
			.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_EN | (interrupt_license == 2 && !pre_throttle_thread ? PMC_SEL_FLAG_INT : 0) | (measure == MEASURE_UPCLOCK || measure == MEASURE_THROTTLE_THROUGHPUT ? PMC_SEL_FLAG_OS : 0),
		}}).u,
	};
	args.msrs[3] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_SEL_3,
		.val = ((union pmc_sel) { .s = {
/*			.event = PM_EVENT_UOPS_ISSUED_STALL_CYCLES,
			.umask = PM_EVENT_UOPS_ISSUED_STALL_CYCLES_UMASK,
			.cmask = PM_EVENT_UOPS_ISSUED_STALL_CYCLES_CMASK,*/
			.event = PM_EVENT_INT_MISC,
			.umask = PM_EVENT_INT_MISC_CLEAR_RESTEER_CYCLES_UMASK,
			.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_EN | PMC_SEL_FLAG_CINV,
		}}).u,
	};

	unsigned char pmc4_umask;
	switch(vector_size) {
		case 256:
			switch(fp_precision) {
				case FP_SP:
				default:
					pmc4_umask = PM_EVENT_FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE_UMASK;
					break;
				case FP_DP:
					pmc4_umask = PM_EVENT_FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE_UMASK;
					break;
			}
			break;
		case 512:
			switch(fp_precision) {
				case FP_SP:
				default:
					pmc4_umask = PM_EVENT_FP_ARITH_INST_RETIRED_512B_PACKED_SINGLE_UMASK;
					break;
				case FP_DP:
					pmc4_umask = PM_EVENT_FP_ARITH_INST_RETIRED_512B_PACKED_DOUBLE_UMASK;
					break;
			}
			break;
	}
	args.msrs[4] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_SEL_4,
		.val = ((union pmc_sel) { .s = {
			.event = PM_EVENT_FP_ARITH_INST_RETIRED,
			.umask = pmc4_umask,
			.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_EN,
		}}).u,
	};


	args.msrs[5] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_SEL_5,
		.val = ((union pmc_sel) { .s = {
			.event = PM_EVENT_UOPS_DISPATCHED_PORT,
			.umask = PM_EVENT_UOPS_DISPATCHED_PORT_0_UMASK,
			.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_EN,
		}}).u,
	};
	args.msrs[6] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_SEL_6,
		.val = ((union pmc_sel) { .s = {
			.event = PM_EVENT_CORE_POWER,
			.umask = PM_EVENT_CORE_POWER_THROTTLE_UMASK,
			.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_EN | ((measure == MEASURE_PRE_THROTTLE_THROUGHPUT || measure == MEASURE_THROTTLE_THROUGHPUT) && !pre_throttle_thread ? PMC_SEL_FLAG_INT : 0),
		}}).u,
	};
	args.msrs[7] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_SEL_7,
		.val = ((union pmc_sel) { .s = {
			.event = PM_EVENT_UOPS_DISPATCHED_PORT,
			.umask = vector_size == 512 ? PM_EVENT_UOPS_DISPATCHED_PORT_5_UMASK : PM_EVENT_UOPS_DISPATCHED_PORT_1_UMASK,
			.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_EN,
		}}).u,
	};
	args.msrs[8] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_0,
		.val = 0,
	};
	args.msrs[9] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_1,
		.val = (((interrupt_license == 1 && measure != MEASURE_PRE_THROTTLE_THROUGHPUT) || (interrupt_license == 2 && measure == MEASURE_NON_AVX_TIME)) ? ULONG_MAX : 0),
	};
	args.msrs[10] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_2,
		.val = (interrupt_license == 2 && measure != MEASURE_PRE_THROTTLE_THROUGHPUT ? ULONG_MAX : 0),
	};
	args.msrs[11] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_3,
		.val = 0,
	};
	args.msrs[12] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_4,
		.val = 0,
	};
	args.msrs[13] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_5,
		.val = 0,
	};
	args.msrs[14] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_6,
		.val = (measure == MEASURE_PRE_THROTTLE_THROUGHPUT || measure == MEASURE_THROTTLE_THROUGHPUT ? ULONG_MAX : 0),
	};
	args.msrs[15] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_PMC_7,
		.val = 0,
	};
	args.msrs[16] = (struct msr_config) {
		.valid = measure == MEASURE_DOWNCLOCK || measure == MEASURE_PRE_THROTTLE_THROUGHPUT || measure == MEASURE_NON_AVX_TIME,
		.addr = MSR_DEBUGCTL,
		.val = DEBUGCTL_FLAG_FREEZE_PERFMON_ON_PMI,
	};
	args.msrs[17] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_FIXED_CTR_CTRL,
		.val = FIXED_CTR_CTRL_0_USR | FIXED_CTR_CTRL_1_USR | FIXED_CTR_CTRL_2_USR,
	};
	args.msrs[18] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_FIXED_CTR_0,
		.val = 0,
	};
	args.msrs[19] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_FIXED_CTR_1,
		.val = 0,
	};
	args.msrs[20] = (struct msr_config) {
		.valid = 1,
		.addr = MSR_FIXED_CTR_2,
		.val = 0,
	};

	/*double zero  = 0.0;
	asm volatile(
		"vbroadcastsd %0, %%zmm0\n"
		"vbroadcastsd %0, %%zmm1\n"
		"vbroadcastsd %0, %%zmm2\n"
		"vbroadcastsd %0, %%zmm3\n"
		"vbroadcastsd %0, %%zmm4\n"
		"vbroadcastsd %0, %%zmm5\n"
		"vbroadcastsd %0, %%zmm6\n"
		"vbroadcastsd %0, %%zmm7\n"
		"vbroadcastsd %0, %%zmm8\n"
		"vbroadcastsd %0, %%zmm9\n"
		"vbroadcastsd %0, %%zmm10\n"
		"vbroadcastsd %0, %%zmm11\n"
		"vbroadcastsd %0, %%zmm12\n"
		"vbroadcastsd %0, %%zmm13\n"
		"vbroadcastsd %0, %%zmm14\n"
		"vbroadcastsd %0, %%zmm15\n"
		"vbroadcastsd %0, %%zmm16\n"
		"vbroadcastsd %0, %%zmm17\n"
		"vbroadcastsd %0, %%zmm18\n"
		"vbroadcastsd %0, %%zmm19\n"
		"vbroadcastsd %0, %%zmm20\n"
		"vbroadcastsd %0, %%zmm21\n"
		"vbroadcastsd %0, %%zmm22\n"
		"vbroadcastsd %0, %%zmm23\n"
		"vbroadcastsd %0, %%zmm24\n"
		"vbroadcastsd %0, %%zmm25\n"
		"vbroadcastsd %0, %%zmm26\n"
		"vbroadcastsd %0, %%zmm27\n"
		"vbroadcastsd %0, %%zmm28\n"
		"vbroadcastsd %0, %%zmm29\n"
		"vbroadcastsd %0, %%zmm30\n"
		"vbroadcastsd %0, %%zmm31\n"
		"vzeroupper\n"
		:: "m" (zero)
	); // notably, without vzeroupper even AVX-2 heavy instructions (lvl1) trigger lvl2 (=> zmm power gating? avx2 instructions executed as avx512 with masking?)*/

	if(!pre_throttle_thread) {
		if(measure == MEASURE_UPCLOCK) {
			args.interrupt_ip = (unsigned long) infinite_loop_memory;
			memcpy(args.interrupt_msrs, args.msrs, sizeof(args.interrupt_msrs));
			args.interrupt_msrs[0] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_SEL_0,
				.val = ((union pmc_sel) { .s = {
					.event = PM_EVENT_CORE_POWER,
					.umask = PM_EVENT_CORE_POWER_LVL0_TURBO_LICENSE_UMASK,
					.flags = PMC_SEL_FLAG_USR | PMC_SEL_FLAG_OS | PMC_SEL_FLAG_INT | PMC_SEL_FLAG_EN,
				}}).u,
			};
			args.interrupt_msrs[17].val = FIXED_CTR_CTRL_0_USR | FIXED_CTR_CTRL_1_USR | FIXED_CTR_CTRL_2_USR | FIXED_CTR_CTRL_0_OS | FIXED_CTR_CTRL_1_OS | FIXED_CTR_CTRL_2_OS; // FIXED_CTR_CTRL
			args.interrupt_msrs[8].val = ULONG_MAX; // PMC_0
			args.interrupt_msrs[9].valid = 0; // PMC_1
			args.interrupt_msrs[10].valid = 0; // PMC_2
			args.interrupt_msrs[16].valid = 1; // DEBUGCTL
		} else if(measure == MEASURE_THROTTLE_THROUGHPUT) {
			args.interrupt_msrs[0] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_0,
				.val = 0
			};
			args.interrupt_msrs[1] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_3,
				.val = 0
			};
			args.interrupt_msrs[2] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_4,
				.val = 0
			};
			args.interrupt_msrs[3] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_5,
				.val = 0
			};
			args.interrupt_msrs[4] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_6,
				.val = 0
			};
			args.interrupt_msrs[5] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_7,
				.val = 0
			};
			args.interrupt_msrs[6] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_DEBUGCTL,
				.val = DEBUGCTL_FLAG_FREEZE_PERFMON_ON_PMI
			};
			args.interrupt_msrs[7] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_FIXED_CTR_0,
				.val = 0
			};
			args.interrupt_msrs[8] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_FIXED_CTR_1,
				.val = 0
			};
			args.interrupt_msrs[9] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_FIXED_CTR_2,
				.val = 0
			};
		} else if(measure == MEASURE_NON_AVX_TIME && interrupt_license == 2) {
			args.interrupt_ip = (unsigned long) avx_memory[1];
			args.interrupt_msrs[0] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_3,
				.val = 0
			};
			args.interrupt_msrs[1] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_4,
				.val = 0
			};
			args.interrupt_msrs[2] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_5,
				.val = 0
			};
			args.interrupt_msrs[3] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_6,
				.val = 0
			};
			args.interrupt_msrs[4] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_PMC_7,
				.val = 0
			};
			args.interrupt_msrs[5] = (struct msr_config) {
				.valid = 1,
				.addr = MSR_DEBUGCTL,
				.val = DEBUGCTL_FLAG_FREEZE_PERFMON_ON_PMI
			};
		}

		pthread_barrier_wait(&barrier);

		// preheat CPU
		while(1) {
			if(alarm_received) break;
		}
	}

	register unsigned long cookie asm("r15") = 0;

	if(pre_throttle_thread && pre_throttle == PRE_THROTTLE_SCALAR) {
		while(1);
	} else {
		// sometimes our kernel component fails to correctly set the IP for reasons I don't entirely understand
		// probably has something to do with kernel preemption or whatever
		// anyway, in case this happens our ioctl() directly returns here and our wait threads get stuck
		// to detect these cases our kernel component sets r15 (callee-saved, which is nice for us) to 3345 if and only if an interrupt occurred
		// so if our cookie in r15 isn't set to 3345 we know that we returned without performing any AVX execution, in this case we exit and the runner scripts will retry
		if(ioctl(ra_dev_fd, IOCTL_SETUP, &args)) {
			fprintf(stderr, "SETUP ioctl failed\n");
			exit(1);
		}

		if(cookie != 3345 && try_elapsed != 1) {
			fprintf(stderr, "invalid cookie value\n");
			ioctl(ra_dev_fd, IOCTL_RESET_WAIT_THREADS);
			exit(1);
		}
	}

	//fprintf(stderr, "exec thread on cpu %i completed\n", cpu);
	return NULL;
}

int main(int argc, char **argv) {
	if(argc < 9) {
		fprintf(stderr, "required arguments: <hex:length> <hex:offset> <dec:interrupt_license> <dec:n_exec_threads> <dec:pre_throttle> <dec:measure> <dec:vector_size> <str:sp/dp>\n");
		exit(1);
	}

	unsigned long length = strtoul(argv[1], NULL, 16);
	unsigned long offset = strtoul(argv[2], NULL, 16);
	interrupt_license = strtoul(argv[3], NULL, 10);
	threads = strtoul(argv[4], NULL, 10);
	pre_throttle = strtoul(argv[5], NULL, 10);
	measure = strtoul(argv[6], NULL, 10);
	vector_size = strtoul(argv[7], NULL, 10);

	if(vector_size != 256 && vector_size != 512) {
		fprintf(stderr, "invalid vector size\n");
		exit(1);
	}

	if(!strcmp(argv[8], "sp")) {
		fp_precision = FP_SP;
	} else if(!strcmp(argv[8], "dp")) {
		fp_precision = FP_DP;
	}

	ra_dev_fd = open("/dev/reclocking_analysis", O_RDWR);
	if(ra_dev_fd < 0) {
		fprintf(stderr, "unable to open device file: %i\n", ra_dev_fd);
		exit(1);
	}

	cpu_set_t set;
	CPU_SET(0, &set);
	if(sched_setaffinity(0, sizeof(cpu_set_t), &set)) {
		fprintf(stderr, "failed to set CPU affinity: %s\n", strerror(errno));
		exit(1);
	}

	if(ioctl(ra_dev_fd, IOCTL_RESET_WAIT_THREADS)) {
		fprintf(stderr, "stuck wait threads were reset\n");
		exit(0);
	}

	if(measure == MEASURE_UPCLOCK)
		map_infinite_loop_code();

	if(measure == MEASURE_NON_AVX_TIME && interrupt_license == 1) {
		avx_instructions = 1;
		map_avx_code(0, LOOP_R12_CMP, argv[0], offset, length);
	} else {
		map_avx_code(0, LOOP_AVX, argv[0], offset, length);
	}

	if(measure == MEASURE_NON_AVX_TIME && interrupt_license == 2) {
		avx_instructions = 1;
		map_avx_code(1, LOOP_R12_CMP, argv[0], offset, length);
	}

	if(measure == MEASURE_NON_AVX_TIME && pre_throttle == PRE_THROTTLE_AVX) {
		map_avx_code(2, LOOP_AVX, argv[0], offset, length);
	}

	unsigned long wait_threads = pre_throttle ? 1 : threads;

	pthread_barrier_init(&barrier, NULL, 2 * wait_threads + 1);

	if(measure == MEASURE_NON_AVX_TIME) {
		for(unsigned long i = 0; i < wait_threads; i++)
			pthread_barrier_init(&thread_barriers[i], NULL, 2);
	}

	for(unsigned long i = 0; i < threads; i++) {
		if(!pre_throttle || i == 0)
			pthread_create(&wait_thread[i], NULL, run_wait_thread, (void*) i);
		pthread_create(&exec_thread[i], NULL, run_exec_thread, (void*) i);
	}

	pthread_barrier_wait(&barrier);
	pthread_barrier_destroy(&barrier);

	if(measure != MEASURE_NON_AVX_TIME)
		pthread_barrier_init(&barrier, NULL, wait_threads);

	/*struct itimerval timer = {
		.it_interval = {
			0, 0
		},
		.it_value = {
			15, 0
		},
	};

	signal(SIGALRM, alarm_handler);
	setitimer(ITIMER_REAL, &timer, NULL);*/
	usleep(500000);
	alarm_received = 1;

	if(measure == MEASURE_NON_AVX_TIME) {
		while(1) {
			usleep(3000);

			unsigned char interrupts_received = 0;
			for(unsigned long i = 0; i < wait_threads; i++) {
				interrupts_received += interrupt_received[i];
			}

			if(interrupts_received != wait_threads)
				ignore_results = 1;

			for(unsigned long i = 0; i < wait_threads; i++) {
				if(!interrupt_received[i])
					continue;

				pthread_barrier_wait(&thread_barriers[i]);
				pthread_barrier_destroy(&thread_barriers[i]);
				pthread_barrier_init(&thread_barriers[i], NULL, 2);
			}

			try_elapsed = 1;

			if(interrupts_received == wait_threads)
				break;

			pthread_barrier_init(&barrier, NULL, 1 + wait_threads + interrupts_received);

			for(unsigned long i = 0; i < (pre_throttle ? 1 : threads); i++) {
				struct timespec wait = {
					.tv_sec = time(NULL) + 2 /* because some idiot decided that pthread_timejoin_np() should use timeouts in absolute values since the epoch, +1 second isn't enough since that may very well be just 1ns away and I am too lazy to properly fix this, so we actually have a timeout within [1,2] here */
				};

				int err;
				if(err = pthread_timedjoin_np(exec_thread[i], NULL, &wait)) {
					// here we basically have the same situation as with the cookie above
					// r12 wasn't correctly set so the exec thread is stuck
					// by using a timeout we detect this situation and exit with an error
					// but first we need to reset our wait thread
					fprintf(stderr, "exec thread is stuck\n");
					ioctl(ra_dev_fd, IOCTL_RESET_WAIT_THREADS);
					exit(1);
				}
			}

			avx_instructions++;
			map_avx_code(interrupt_license == 1 ? 0 : 1, LOOP_R12_CMP, argv[0], offset, length);

			try_elapsed = 0;
			alarm_received = 0;
			ignore_results = 0;

			for(unsigned long i = 0; i < wait_threads; i++) {
				if(!interrupt_received[i])
					continue;

				pthread_join(wait_thread[i], NULL);
				pthread_create(&wait_thread[i], NULL, run_wait_thread, (void*) i);
			}

			memset((void*) interrupt_received, 0, 64 * sizeof(char));

			for(unsigned long i = 0; i < wait_threads; i++) {
				pthread_create(&exec_thread[i], NULL, run_exec_thread, (void*) i);
			}

			pthread_barrier_wait(&barrier);
			pthread_barrier_destroy(&barrier);

			usleep(3000); // we don't really require pre-heating, however, to get reliable results we need some more delay
			alarm_received = 1;
		}
	}

	for(unsigned long i = 0; i < (pre_throttle ? 1 : threads); i++) {
		pthread_join(exec_thread[i], NULL);
		pthread_join(wait_thread[i], NULL);
	}

	printf("--success--\n");

	return 0;
}
