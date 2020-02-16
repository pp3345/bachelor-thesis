#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpu.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/preempt.h>
#include <linux/ptrace.h>
#include <linux/regset.h>
#include <asm/cpu.h>
#include <asm/desc.h>
#include <asm/msr.h>
#include <asm/processor.h>
#include <asm/traps.h>
#include <asm/user_64.h>

#include "../include/shared.h"

#define REGSET_GENERAL 0 /* arch/x86/kernel/ptrace.c -> enum x86_regset */

/* TODO be less fragile (how to select free vector on Linux >= 4.15?) */
#define INTERRUPT_VECTOR 0xee

static u8 n_pmc __read_mostly = 8;
static u8 avxfreq __read_mostly = 0;

/* this might clash with other modules */
#define DEVICE_MAJOR 234

struct sw_counters {
	u64 lvl0_cycles;
	u64 lvl1_cycles;
	u64 lvl2_cycles;
	u64 fp512_sp_retired;
	u64 fp512_dp_retired;
	u64 fp256_sp_retired;
	u64 fp256_dp_retired;
	u64 cycles_total;
};

DEFINE_PER_CPU(unsigned long, previous_ip);
DEFINE_PER_CPU(unsigned long, previous_r12);
DEFINE_PER_CPU(struct interrupt_result, interrupt_result);
DEFINE_PER_CPU(struct task_struct*, waiting_task);
DEFINE_PER_CPU(char, interrupt_action);
DEFINE_PER_CPU(struct msr_config[32], interrupt_msrs);
DEFINE_PER_CPU(unsigned long, interrupt_ip);
DEFINE_PER_CPU(unsigned int, platform_energy_start);
DEFINE_PER_CPU(unsigned int, package_energy_start);
DEFINE_PER_CPU(unsigned long, productive_cycles_start);
DEFINE_PER_CPU(union pmc_sel[MAX_PMCS], pmc_configs);
DEFINE_PER_CPU(struct sw_counters, sw_counters);
DEFINE_PER_CPU(avxfreq_counters*, bare_counters);

#define is_avxfreq_pmc(pmc) (avxfreq && pmc < AVXFREQ_PMCS)
#define is_avxfreq_msr(msr) (avxfreq && ((msr >= MSR_PMC_0 && msr <= MSR_PMC_0 + AVXFREQ_PMCS - 1) || (msr >= MSR_PMC_SEL_0 && msr <= MSR_PMC_SEL_0 + AVXFREQ_PMCS - 1) || msr == MSR_FIXED_CTR_CTRL || msr == MSR_DEBUGCTL))
#define is_pmc_msr(msr) (msr >= MSR_PMC_0 && msr <= MSR_PMC_7)
#define is_pmc_sel_msr(msr) (msr >= MSR_PMC_SEL_0 && msr <= MSR_PMC_SEL_7)
#define msr_to_pmc(msr) (msr - MSR_PMC_0)
#define pmc_to_msr(pmc) (pmc + MSR_PMC_0)
#define msr_to_pmc_sel(msr) (msr - MSR_PMC_SEL_0)
#define pmc_sel_to_msr(pmc) (pmc + MSR_PMC_SEL_0)

static inline unsigned int wrap_counter(unsigned int start, unsigned int end) {
	if(end < start)
		return UINT_MAX - start + end;
	else
		return end - start;
}

static inline unsigned long wrap_counter_long(unsigned long start, unsigned long end) {
	if(end < start)
		return ULONG_MAX - start + end;
	else
		return end - start;
}

static inline u64 read_pmc(u8 pmc) {
	if(avxfreq) {
		union pmc_sel config = this_cpu_read(pmc_configs[pmc]);
		switch(config.s.event) {
			case PM_EVENT_CORE_POWER:
				switch(config.s.umask) {
					case PM_EVENT_CORE_POWER_LVL0_TURBO_LICENSE_UMASK:
						return this_cpu_ptr(&sw_counters)->lvl0_cycles;
					case PM_EVENT_CORE_POWER_LVL1_TURBO_LICENSE_UMASK:
						return this_cpu_ptr(&sw_counters)->lvl1_cycles;
					case PM_EVENT_CORE_POWER_LVL2_TURBO_LICENSE_UMASK:
						return this_cpu_ptr(&sw_counters)->lvl2_cycles;
				}
				break;
			case PM_EVENT_FP_ARITH_INST_RETIRED:
				switch(config.s.umask) {
					case PM_EVENT_FP_ARITH_INST_RETIRED_512B_PACKED_SINGLE_UMASK:
						return this_cpu_ptr(&sw_counters)->fp512_sp_retired;
					case PM_EVENT_FP_ARITH_INST_RETIRED_512B_PACKED_DOUBLE_UMASK:
						return this_cpu_ptr(&sw_counters)->fp512_dp_retired;
					case PM_EVENT_FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE_UMASK:
						return this_cpu_ptr(&sw_counters)->fp256_sp_retired;
					case PM_EVENT_FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE_UMASK:
						return this_cpu_ptr(&sw_counters)->fp256_dp_retired;
				}
				break;
		}
	}

	if(is_avxfreq_pmc(pmc)) {
		return 123456789; // we have no choice but to fake some value here
	}

	return native_read_msr(MSR_PMC_0 + pmc);
}

static inline u64 read_fixed_pmc(u8 pmc) {
	if(avxfreq && pmc == 1)
		return this_cpu_ptr(&sw_counters)->cycles_total;

	return native_read_msr(MSR_FIXED_CTR_0 + pmc);
}

static inline void reset_pmi(void) {
	int ret;

	if(avxfreq)
		return;

	if(unlikely(ret = wrmsrl_safe(MSR_PERF_GLOBAL_OVF_CTRL, 0x8000000000000ffull))) {
		pr_warn("failed to reset PMU on CPU %i: wrmrsl @0x%x = %i\n", smp_processor_id(), MSR_PERF_GLOBAL_OVF_CTRL, ret);
	}

	if(unlikely(ret = wrmsrl_safe(MSR_DEBUGCTL, 0))) {
		pr_warn("failed to reset PMU on CPU %i: wrmrsl @0x%x = %i\n", smp_processor_id(), MSR_DEBUGCTL, ret);
	}
}

static void reset_pmu(void *info) {
	u32 pmc;
	int ret;

	for(pmc = 0; pmc < n_pmc; pmc++) {
		if(is_avxfreq_pmc(pmc))
			continue;

		if(unlikely(ret = wrmsrl_safe(MSR_PMC_SEL_0 + pmc, 0x0ull))) {
			pr_warn("failed to reset PMU on CPU %i: wrmrsl @0x%x = %i\n", smp_processor_id(), MSR_PMC_SEL_0 + pmc, ret);
		}

		if(unlikely(ret = wrmsrl_safe(MSR_PMC_0 + pmc, 0x0ull))) {
			pr_warn("failed to reset PMU on CPU %i: wrmrsl @0x%x = %i\n", smp_processor_id(), MSR_PMC_0 + pmc, ret);
		}
	}

	reset_pmi();
}

static inline void set_msrs(struct msr_config msrs[]) {
	int i;

	for(i = 0; i < 32; i++) {
		struct msr_config msr = msrs[i];
		if(msr.valid != 1)
			continue;

		if(avxfreq) {
			if(is_pmc_sel_msr(msr.addr)) {
				this_cpu_write(pmc_configs[msr_to_pmc_sel(msr.addr)], (union pmc_sel) msr.val);
			} else if(is_pmc_msr(msr.addr)) {
				union pmc_sel config = this_cpu_read(pmc_configs[msr_to_pmc(msr.addr)]);

				switch(config.s.event) {
					case PM_EVENT_CORE_POWER:
						switch(config.s.umask) {
							case PM_EVENT_CORE_POWER_LVL0_TURBO_LICENSE_UMASK:
								this_cpu_ptr(&sw_counters)->lvl0_cycles = msr.val;
								continue;
							case PM_EVENT_CORE_POWER_LVL1_TURBO_LICENSE_UMASK:
								this_cpu_ptr(&sw_counters)->lvl1_cycles = msr.val;
								continue;
							case PM_EVENT_CORE_POWER_LVL2_TURBO_LICENSE_UMASK:
								this_cpu_ptr(&sw_counters)->lvl2_cycles = msr.val;
								continue;
						}
						break;
					case PM_EVENT_FP_ARITH_INST_RETIRED:
						switch(config.s.umask) {
							case PM_EVENT_FP_ARITH_INST_RETIRED_512B_PACKED_SINGLE_UMASK:
								this_cpu_ptr(&sw_counters)->fp512_sp_retired = msr.val;
								continue;
							case PM_EVENT_FP_ARITH_INST_RETIRED_512B_PACKED_DOUBLE_UMASK:
								this_cpu_ptr(&sw_counters)->fp512_dp_retired = msr.val;
								continue;
							case PM_EVENT_FP_ARITH_INST_RETIRED_256B_PACKED_SINGLE_UMASK:
								this_cpu_ptr(&sw_counters)->fp256_sp_retired = msr.val;
								continue;
							case PM_EVENT_FP_ARITH_INST_RETIRED_256B_PACKED_DOUBLE_UMASK:
								this_cpu_ptr(&sw_counters)->fp256_dp_retired = msr.val;
								continue;
						}
						break;
				}
			} else if(msr.addr == MSR_FIXED_CTR_1) {
				this_cpu_ptr(&sw_counters)->cycles_total = msr.val;

				if(msr.val == 0)
					avxfreq_reset_cycle_counter();

				continue;
			}
		}

		if(is_avxfreq_msr(msr.addr)) {
			continue;
		}

		wrmsrl(msr.addr, msr.val);
	}
}

/*
static inline int _set_task_register(struct task_struct *task, unsigned int offset, unsigned long value) {
	const struct user_regset *regset = &task_user_regset_view(task)->regsets[REGSET_GENERAL];

	return regset->set(task, regset, offset, sizeof(unsigned long), NULL, &value);
}

#define set_task_register(task, reg, val) _set_task_register(task, offsetof(struct user_regs_struct, reg), val)
*/
static long ra_ioctl_setup(unsigned long arg) {
	struct setup_args args;
	struct pt_regs *regs = task_pt_regs(current);
	struct interrupt_result *result_ptr = this_cpu_ptr(&interrupt_result);
	int res;

	//pr_info("setup ioctl on cpu %i\n", smp_processor_id());

	if(copy_from_user(&args, (void*) arg, sizeof(struct setup_args))) {
		pr_warn("invalid setup_args passed\n");
		return -EINVAL;
	}

	memcpy(this_cpu_ptr(&interrupt_msrs), &args.interrupt_msrs, sizeof(args.interrupt_msrs));
	this_cpu_write(interrupt_action, args.interrupt_action);
	this_cpu_write(interrupt_ip, args.interrupt_ip);
	this_cpu_write(previous_ip, regs->ip);
	this_cpu_write(previous_r12, regs->r12);
	//pr_info("cpu%i: jump from %lx to %lx (setup) (tid=%i)", smp_processor_id(), regs->ip, args.ip, current->pid);
	regs->ip = args.ip;
	if(args.r12 != 0)
		regs->r12 = args.r12;
/*
	set_task_register(current, ip, args.ip);
	if(args.r12 != 0)
		set_task_register(current, r12, args.r12);
*/
	this_cpu_write(package_energy_start, (unsigned int) native_read_msr(MSR_PKG_ENERGY_STATUS));
	this_cpu_write(platform_energy_start, (unsigned int) native_read_msr(MSR_PLATFORM_ENERGY_COUNTER));
	this_cpu_write(productive_cycles_start, native_read_msr(MSR_PPERF));
	set_msrs(args.msrs);
	result_ptr->valid = 0;
	result_ptr->tsc_start = rdtsc_ordered();

	return 0;
}

static long ra_ioctl_wait_for_interrupt(unsigned long arg) {
	struct interrupt_result input;

	//pr_info("wait_for_interrupt ioctl on cpu %i\n", smp_processor_id());

	if(copy_from_user(&input, (void*) arg, sizeof(struct interrupt_result))) {
		pr_warn("invalid interrupt_result area passed\n");
		return -EINVAL;
	}

	per_cpu(waiting_task, input.cpu) = current;
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule();

	//pr_info("wait_for_interrupt ioctl returning on cpu %i\n", smp_processor_id());

	if(!per_cpu_ptr(&interrupt_result, input.cpu)->valid)
		return -EINTR;

	if(copy_to_user((void*) arg, per_cpu_ptr(&interrupt_result, input.cpu), sizeof(struct interrupt_result))) {
		pr_warn("invalid interrupt_result area passed\n");
		return -EINVAL;
	}

	per_cpu_ptr(&interrupt_result, input.cpu)->valid = 0;

	return 0;
}

static long ra_ioctl_reset_wait_threads(unsigned long arg) {
	int cpu;
	int n = 0;

	for_each_online_cpu(cpu) {
		if(per_cpu(waiting_task, cpu) != NULL) {
			wake_up_process(per_cpu(waiting_task, cpu));
			per_cpu(waiting_task, cpu) = NULL;
			n++;
		}
	}

	return n;
}

static long ra_ioctl(struct file *file, unsigned int ioc, unsigned long arg) {
	long ret;

	switch(ioc) {
		case IOCTL_SETUP:
			ret = ra_ioctl_setup(arg);
			break;
		case IOCTL_WAIT_FOR_INTERRUPT:
			ret = ra_ioctl_wait_for_interrupt(arg);
			break;
		case IOCTL_RESET_WAIT_THREADS:
			ret = ra_ioctl_reset_wait_threads(arg);
			break;
		default:
			ret = -EINVAL;
	}

	return ret;
}

static int ra_open(struct inode *inode, struct file *file) {
	return 0;
}

static char *ra_devnode(struct device *dev, umode_t *mode) {
	if(mode != NULL) {
		*mode = 0666;
	}
	return kstrdup("reclocking_analysis", GFP_KERNEL);
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = ra_open,
	.unlocked_ioctl = ra_ioctl,
	.compat_ioctl = ra_ioctl,
};
static struct class *class;

asmlinkage void reclocking_analysis_interrupt(void);
dotraplinkage void do_reclocking_analysis_interrupt(struct pt_regs *unused) {
	unsigned long tsc = rdtsc_ordered();
	struct pt_regs *regs = task_pt_regs(current);
	union {
		struct perf_global_status s;
		u64 u;
	} c;
	struct interrupt_result *result_ptr = this_cpu_ptr(&interrupt_result);

	//pr_info("received interrupt on cpu %i\n", smp_processor_id());

	switch(this_cpu_read(interrupt_action)) {
		case INTERRUPT_ACTION_WAKE_WAIT_THREAD: {
			if(this_cpu_read(waiting_task) != NULL) {
				int i;
				int cpu = smp_processor_id();

				result_ptr->power.platform_energy = wrap_counter(this_cpu_read(platform_energy_start), (unsigned int) native_read_msr(MSR_PLATFORM_ENERGY_COUNTER));
				result_ptr->power.package_energy = wrap_counter(this_cpu_read(package_energy_start), (unsigned int) native_read_msr(MSR_PKG_ENERGY_STATUS));
				result_ptr->power.perf_limit_reasons = native_read_msr(MSR_CORE_PERF_LIMIT_REASONS);
				result_ptr->power.ring_perf_limit_reasons = native_read_msr(MSR_RING_PERF_LIMIT_REASONS);

				if(!avxfreq)
					result_ptr->performance.request.u = native_read_msr(MSR_HWP_REQUEST);
				result_ptr->performance.productive_cycles = wrap_counter_long(this_cpu_read(productive_cycles_start), native_read_msr(MSR_PPERF));

				result_ptr->cpu = cpu;
				result_ptr->tsc_end = tsc;

				for(i = 0; i < n_pmc; i++) {
					result_ptr->pmcs[i] = read_pmc(i);
				}

				result_ptr->fixed_counters.instructions_retired = read_fixed_pmc(0);
				result_ptr->fixed_counters.cpu_cycles_unhalted = read_fixed_pmc(1);
				result_ptr->fixed_counters.cpu_cycles_unhalted_tsc = read_fixed_pmc(2);

				c.u = native_read_msr(MSR_PERF_GLOBAL_STATUS);
				result_ptr->perf_global_status = c.s;
				result_ptr->ip = regs->ip;
				result_ptr->valid = 1;

				//pr_info("wake tid=%i\n", this_cpu_read(waiting_task)->pid);
				wake_up_process(this_cpu_read(waiting_task));

				if(current && current->parent == this_cpu_read(waiting_task)->parent) { // if our userspace component crashed we might end up sending other processes to bogus IPs
					regs->ip = this_cpu_read(previous_ip);
					regs->r12 = this_cpu_read(previous_r12);
					regs->r15 = 3345;
					/*set_task_register(current, r15, 3345);
					set_task_register(current, ip, this_cpu_read(previous_ip));
					set_task_register(current, r12, this_cpu_read(previous_r12));*/
					//pr_info("cpu%i: jump to %lx (wake_wait_thread)\n", cpu, regs->ip);
					this_cpu_write(previous_ip, 0);
				}

				this_cpu_write(waiting_task, NULL);
			}

			this_cpu_write(interrupt_action, 0);
			reset_pmu(NULL);
		} break;
		case INTERRUPT_ACTION_GOTO:
			regs->ip = this_cpu_read(interrupt_ip);
			//set_task_register(current, ip, this_cpu_read(interrupt_ip));
			//pr_info("cpu%i: jump to %lx (goto)\n", smp_processor_id(), regs->ip);
			// break missing intentionally
		case INTERRUPT_ACTION_SET_MSRS:
			reset_pmi();
			result_ptr->tsc_start = tsc;
			this_cpu_write(interrupt_action, INTERRUPT_ACTION_WAKE_WAIT_THREAD);
			set_msrs(*this_cpu_ptr(&interrupt_msrs));
			break;
		default:
			// crash userspace
			/*regs->ip = 0xdeadbeef;
			pr_warn("invalid interrupt action\n");*/
			// ^ this will kill random processes if we don't exclusively own the interrupt vector we use (uncomment if you like russian roulette)
			break;
	}

	if(!avxfreq) {
		apic_write(APIC_LVTPC, INTERRUPT_VECTOR);
		apic_eoi();
	}
}

static void ra_avxfreq_listener(u8 from, u8 to) {
	int i;
	struct sw_counters *counters = this_cpu_ptr(&sw_counters);
	avxfreq_counters *src = this_cpu_read(bare_counters);

	switch(from) {
		case 0:
			counters->lvl0_cycles += src->cycles;
			break;
		case 1:
			counters->lvl1_cycles += src->cycles;
			break;
		case 2:
			counters->lvl2_cycles += src->cycles;
			break;
	}

	counters->cycles_total += src->cycles;

	switch(to) {
		case 0:
			counters->lvl0_cycles++;
			break;
		case 1:
			counters->lvl1_cycles++;
			break;
		case 2:
			counters->lvl2_cycles++;
			break;
	}

	counters->fp256_sp_retired += src->fp256_sp_retired;
	counters->fp256_dp_retired += src->fp256_dp_retired;
	counters->fp512_sp_retired += src->fp512_sp_retired;
	counters->fp512_dp_retired += src->fp512_dp_retired;

	for(i = 0; i < n_pmc; i++) {
		union pmc_sel config = this_cpu_read(pmc_configs[i]);

		if(config.s.event != PM_EVENT_CORE_POWER)
			continue;

		if(!(config.s.flags & PMC_SEL_FLAG_INT))
			continue;

		switch(config.s.umask) {
			case PM_EVENT_CORE_POWER_LVL0_TURBO_LICENSE_UMASK:
				if(to != 0)
					continue;
				break;
			case PM_EVENT_CORE_POWER_LVL1_TURBO_LICENSE_UMASK:
				if(to != 1)
					continue;
				break;
			case PM_EVENT_CORE_POWER_LVL2_TURBO_LICENSE_UMASK:
				if(to != 2)
					continue;
				break;
			default:
				pr_warn("unsupported fake interrupt requested\n");
				continue;
		}

		do_reclocking_analysis_interrupt(NULL);
	}
}

static void install_interrupt(void *info) {
	gate_desc desc;
	gate_desc *idt = (gate_desc*) kallsyms_lookup_name("idt_table");
	u32 current_vector = apic_read(APIC_LVTPC);

	pr_info("installing interrupt on cpu %i, current vector is %u\n", smp_processor_id(), current_vector);

	if(idt == NULL) {
		pr_err("failed to get idt_table\n");
		return;
	}

	pack_gate(&desc, GATE_INTERRUPT, (unsigned long) reclocking_analysis_interrupt, 0, 0, 0);
	write_idt_entry(idt, INTERRUPT_VECTOR, &desc);

	apic_write(APIC_LVTPC, INTERRUPT_VECTOR);
}

static int __init reclocking_analysis_init(void) {
	int ret, cpu;
	struct device *dev;

	n_pmc = (cpuid_eax(0xA) >> 8) & 0b11111111;
	pr_info("CPU has %hhu performance counters\n", n_pmc);

	if(avxfreq_is_enabled()) {
		pr_info("Hooking up AVXFreq listener\n");
		avxfreq_set_license_transition_listener(ra_avxfreq_listener);
		avxfreq = 1;

		for_each_online_cpu(cpu) {
			per_cpu(bare_counters, cpu) = avxfreq_get_counters(cpu);
		}
	}

	on_each_cpu(reset_pmu, NULL, 1);

	if((ret = register_chrdev(DEVICE_MAJOR, "reclocking_analysis", &fops))) {
		pr_err("unable to register character device: %i\n", ret);
		return -EIO;
	}

	class = class_create(THIS_MODULE, "reclocking_analysis");
	if(IS_ERR(class)) {
		pr_err("unable to create device class: %li\n", PTR_ERR(class));
		unregister_chrdev(DEVICE_MAJOR, "reclocking_analysis");
		return PTR_ERR(class);
	}

	class->devnode = ra_devnode;
	dev = device_create(class, NULL, MKDEV(DEVICE_MAJOR, 0), NULL, "reclocking_analysis");
	if(IS_ERR(dev)) {
		pr_err("unable to create device: %li\n", PTR_ERR(dev));
		unregister_chrdev(DEVICE_MAJOR, "reclocking_analysis");
		class_destroy(class);
		return PTR_ERR(dev);
	}

	if(!avxfreq) {
		cpus_read_lock();
		on_each_cpu(install_interrupt, NULL, 1);
		cpus_read_unlock();
	}

	return 0;
}

static void __exit reclocking_analysis_exit(void) {
	on_each_cpu(reset_pmu, NULL, 1);
	device_destroy(class, MKDEV(DEVICE_MAJOR, 0));
	class_destroy(class);
	unregister_chrdev(DEVICE_MAJOR, "reclocking_analysis");
}

module_init(reclocking_analysis_init);
module_exit(reclocking_analysis_exit);

MODULE_AUTHOR("Yussuf Khalil <yussuf.khalil@student.kit.edu>");
MODULE_DESCRIPTION("Module to help analyzing AVX reclocking behavior");
MODULE_LICENSE("GPL");
