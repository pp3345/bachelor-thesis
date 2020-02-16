#include <linux/types.h>
#include <linux/ioctl.h>

struct msr_config {
	unsigned char valid;
	unsigned int addr;
	unsigned long val;
};

union pmc_sel {
	struct {
		unsigned char event;
		unsigned char umask;
		unsigned char flags;
		unsigned char cmask;
		unsigned int reserved;
	} s;
	unsigned long u;
};

struct setup_args {
	struct msr_config msrs[32];
	unsigned long ip;
	unsigned long r12;
	char interrupt_action;
	struct msr_config interrupt_msrs[32];
	unsigned long interrupt_ip;
};

struct perf_global_status {
	struct {
		unsigned char pmc0:1;
		unsigned char pmc1:1;
		unsigned char pmc2:1;
		unsigned char pmc3:1;
		unsigned char pmc4:1;
		unsigned char pmc5:1;
		unsigned char pmc6:1;
		unsigned char pmc7:1;
	} overflow;
	unsigned char reserved[3];
	struct {
		unsigned char ctr0:1;
		unsigned char ctr1:1;
		unsigned char ctr2:1;
		unsigned char reserved:5;
	} fixed_overflow;
	unsigned char reserved2;
	unsigned char perf_metric_overflow:1;
	unsigned char reserved3:6;
	unsigned char trace_topa_pmi:1;
	unsigned char reserved4:2;
	unsigned char lbr_frozen:1;
	unsigned char counters_frozen:1;
	unsigned char contains_secure_enclave:1;
	unsigned char uncore_overflow:1;
	unsigned char ds_save_overflow:1;
	unsigned char conditions_changed:1;
};

struct interrupt_result {
	int cpu;
	unsigned long pmcs[8];
	struct perf_global_status perf_global_status;
	struct {
		unsigned long instructions_retired;
		unsigned long cpu_cycles_unhalted;
		unsigned long cpu_cycles_unhalted_tsc;
	} fixed_counters;
	struct {
		unsigned int platform_energy; // MSR_PLATFORM_ENERGY_COUNTER
		unsigned int package_energy; // MSR_PKG_ENERGY_STATUS
		unsigned int perf_limit_reasons; // MSR_CORE_PERF_LIMIT_REASONS
		unsigned int ring_perf_limit_reasons; // MSR_RING_PERF_LIMIT_REASONS
	} power;
	struct {
		union {
			unsigned long u;
			struct {
				unsigned char min;
				unsigned char max;
				unsigned char desired;
				unsigned char pref;
				unsigned char activity_window;
				unsigned char package_control:1;
			} s;
		} request; // IA32_HWP_REQUEST
		unsigned long productive_cycles; // MSR_PPERF
	} performance;
	unsigned long ip;
	unsigned long freq;
	unsigned long tsc_start;
	unsigned long tsc_end;
	char valid:1;
};

#define IOCTL_SETUP _IOW(0xDE, 0x80, struct setup_args)
#define IOCTL_WAIT_FOR_INTERRUPT _IOWR(0xDE, 0x81, struct interrupt_result)
#define IOCTL_RESET_WAIT_THREADS _IO(0xDE, 0x8)

#define MSR_PMC_SEL_0 0x186
#define MSR_PMC_SEL_1 (MSR_PMC_SEL_0 + 1)
#define MSR_PMC_SEL_2 (MSR_PMC_SEL_0 + 2)
#define MSR_PMC_SEL_3 (MSR_PMC_SEL_0 + 3)
#define MSR_PMC_SEL_4 (MSR_PMC_SEL_0 + 4)
#define MSR_PMC_SEL_5 (MSR_PMC_SEL_0 + 5)
#define MSR_PMC_SEL_6 (MSR_PMC_SEL_0 + 6)
#define MSR_PMC_SEL_7 (MSR_PMC_SEL_0 + 7)
#define MSR_PMC_0 0xc1
#define MSR_PMC_1 (MSR_PMC_0 + 1)
#define MSR_PMC_2 (MSR_PMC_0 + 2)
#define MSR_PMC_3 (MSR_PMC_0 + 3)
#define MSR_PMC_4 (MSR_PMC_0 + 4)
#define MSR_PMC_5 (MSR_PMC_0 + 5)
#define MSR_PMC_6 (MSR_PMC_0 + 6)
#define MSR_PMC_7 (MSR_PMC_0 + 7)
#define MSR_PERF_GLOBAL_STATUS 0x38e
#define MSR_PERF_GLOBAL_OVF_CTRL 0x390
#define MSR_DEBUGCTL 0x1d9
#define MSR_FIXED_CTR_0 0x309 // counts total retired macro instructions
#define MSR_FIXED_CTR_1 0x30a // counts cycles while core is not in a halt state
#define MSR_FIXED_CTR_2 0x30b // counts cycles while core is not in a halt state with respect to TSC
#define MSR_FIXED_CTR_CTRL 0x38d
//#define MSR_PKG_ENERGY_STATUS 0x611
#define MSR_PLATFORM_ENERGY_COUNTER 0x64D
//#define MSR_PPERF 0x64E
#undef MSR_CORE_PERF_LIMIT_REASONS // the definition provided by msr-index.h is only valid for Haswell and Broadwell and wrong for Skylake+
#define MSR_CORE_PERF_LIMIT_REASONS 0x64F
//#define MSR_RING_PERF_LIMIT_REASONS 0x6B1
//#define MSR_HWP_REQUEST 0x774

#define MAX_PMCS 8

#define PMC_SEL_FLAG_USR  0b00000001 // count in ring > 0
#define PMC_SEL_FLAG_OS   0b00000010 // count in ring = 0
#define PMC_SEL_FLAG_EDGE 0b00000100 // edge-triggered
#define PMC_SEL_FLAG_PC   0b00001000 // enable pin control
#define PMC_SEL_FLAG_INT  0b00010000 // enable overflow interrupts
#define PMC_SEL_FLAG_AT   0b00100000 // anythread/cumulative counters for SMT
#define PMC_SEL_FLAG_EN   0b01000000 // enable
#define PMC_SEL_FLAG_CINV 0b10000000 // invert cmask

#define PM_EVENT_CORE_POWER 0x28
#define PM_EVENT_CORE_POWER_LVL0_TURBO_LICENSE_UMASK 0x07
#define PM_EVENT_CORE_POWER_LVL1_TURBO_LICENSE_UMASK 0x18
#define PM_EVENT_CORE_POWER_LVL2_TURBO_LICENSE_UMASK 0x20
#define PM_EVENT_CORE_POWER_THROTTLE_UMASK 0x40

#define PM_EVENT_UOPS_ISSUED_STALL_CYCLES 0x0E
#define PM_EVENT_UOPS_ISSUED_STALL_CYCLES_UMASK 0x01
#define PM_EVENT_UOPS_ISSUED_STALL_CYCLES_CMASK 0x1

#define PM_EVENT_FP_ARITH_INST_RETIRED 0xc7
#define PM_EVENT_FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE_UMASK 0x10
#define PM_EVENT_FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE_UMASK 0x20
#define PM_EVENT_FP_ARITH_INST_RETIRED_512B_PACKED_DOUBLE_UMASK 0x40
#define PM_EVENT_FP_ARITH_INST_RETIRED_512B_PACKED_SINGLE_UMASK 0x80

#define PM_EVENT_UOPS_DISPATCHED_PORT 0xa1
#define PM_EVENT_UOPS_DISPATCHED_PORT_0_UMASK 0x01
#define PM_EVENT_UOPS_DISPATCHED_PORT_1_UMASK 0x02
#define PM_EVENT_UOPS_DISPATCHED_PORT_2_UMASK 0x04
#define PM_EVENT_UOPS_DISPATCHED_PORT_3_UMASK 0x08
#define PM_EVENT_UOPS_DISPATCHED_PORT_4_UMASK 0x10
#define PM_EVENT_UOPS_DISPATCHED_PORT_5_UMASK 0x20
#define PM_EVENT_UOPS_DISPATCHED_PORT_6_UMASK 0x40
#define PM_EVENT_UOPS_DISPATCHED_PORT_7_UMASK 0x80

#define PM_EVENT_CYCLE_ACTIVITY 0xA3
#define PM_EVENT_CYCLE_ACTIVITY_STALLS_TOTAL_UMASK 0x04

#define PM_EVENT_RESERVATION_STATION 0x5E
#define PM_EVENT_RESERVATION_STATION_EMPTY_CYCLES_UMASK 0x01

#define PM_EVENT_INT_MISC 0x0D
#define PM_EVENT_INT_MISC_RECOVERY_CYCLES_UMASK 0x01
#define PM_EVENT_INT_MISC_CLEAR_RESTEER_CYCLES_UMASK 0x80

#define DEBUGCTL_FLAG_FREEZE_PERFMON_ON_PMI (1 << 12)

#define FIXED_CTR_CTRL_0_OS  (1 << 0)
#define FIXED_CTR_CTRL_0_USR (1 << 1)
#define FIXED_CTR_CTRL_1_OS  (1 << 4)
#define FIXED_CTR_CTRL_1_USR (1 << 5)
#define FIXED_CTR_CTRL_2_OS  (1 << 8)
#define FIXED_CTR_CTRL_2_USR (1 << 9)

#define INTERRUPT_ACTION_WAKE_WAIT_THREAD 1
#define INTERRUPT_ACTION_GOTO 2
#define INTERRUPT_ACTION_SET_MSRS 3
