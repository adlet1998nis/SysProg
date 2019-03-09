#include <linux/module.h>       
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <asm/current.h>
#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/sched/stat.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <linux/sched/cputime.h>
#include <linux/tick.h>

u64 get_idle_time(int cpu){
	u64 idle = 0;
	idle = kcpustat_cpu(cpu).cpustat[CPUTIME_IDLE];
	return idle;
}

u64 get_iowait_time(int cpu){
	u64 iowait = 0;
	iowait = kcpustat_cpu(cpu).cpustat[CPUTIME_IOWAIT];
	return iowait;
}

u64 get_user(int cpu){
	u64 user = 0;
	user = kcpustat_cpu(cpu).cpustat[CPUTIME_USER];
	return user;
}

u64 get_nice(int cpu){
	u64 nice = 0;
	nice = kcpustat_cpu(cpu).cpustat[CPUTIME_NICE];
	return nice;
}

u64 get_system(int cpu){
	u64 system = 0;
	system = kcpustat_cpu(cpu).cpustat[CPUTIME_SYSTEM];
	return system;
}

u64 get_irq(int cpu){
	u64 irq = 0;
	irq = kcpustat_cpu(cpu).cpustat[CPUTIME_IRQ];
	return irq;
}

u64 get_softirq(int cpu){
	u64 softirq = 0;
	softirq = kcpustat_cpu(cpu).cpustat[CPUTIME_SOFTIRQ];
	return softirq;
}

u64 get_steal(int cpu){
	u64 steal = 0;
	steal = kcpustat_cpu(cpu).cpustat[CPUTIME_STEAL];
	return steal;
}

u64 get_guest(int cpu){
	u64 guest = 0;
	guest = kcpustat_cpu(cpu).cpustat[CPUTIME_GUEST];
	return guest;
}

u64 get_guest_nice(int cpu){
	u64 guest_nice = 0;
	guest_nice = kcpustat_cpu(cpu).cpustat[CPUTIME_GUEST_NICE];
	return guest_nice;
}

u64 nsec_to_clock_t(u64 x){
#if (NSEC_PER_SEC % USER_HZ) == 0
	return div_u64(x, NSEC_PER_SEC / USER_HZ);
#elif (USER_HZ % 512) == 0
	return div_u64(x * USER_HZ / 512, NSEC_PER_SEC / 512);
#else
	return div_u64(x * 9, (9ull * NSEC_PER_SEC + (USER_HZ / 2)) / USER_HZ);
#endif
}

u64 calc_cpu_usage(u64 user, u64 nice, u64 system, u64 idle, u64 iowait, u64 irq, u64 softirq, u64 steal){
	u64 CPU_time_since_boot, CPU_idle_time_since_boot, CPU_usage_time_since_boot, CPU_percentage; 
	CPU_time_since_boot = user + nice + system + idle + iowait + irq + softirq + steal;
	CPU_idle_time_since_boot = idle + iowait;
	CPU_usage_time_since_boot = CPU_time_since_boot - CPU_idle_time_since_boot;
	CPU_percentage = ((CPU_usage_time_since_boot * 100) / CPU_time_since_boot);
	return CPU_percentage;
}

void print_info(void) {
	int i;
	u64 CPU_usage_percent[3];
	u64 total_CPU_usage_percent = 0;
	u64 cnt = 0;
	u64 total_idle = 0, total_iowait = 0, total_user = 0, total_nice = 0, total_system = 0, total_irq = 0, total_softirq = 0, total_steal = 0, total_guest = 0, total_guest_nice = 0;
	u64 cur_idle = 0, cur_iowait = 0, cur_user = 0, cur_nice = 0, cur_system = 0, cur_irq = 0, cur_softirq = 0, cur_steal = 0, cur_guest = 0, cur_guest_nice = 0;
	printk(KERN_INFO "Begin");
	for_each_possible_cpu(i){
		cur_idle = get_idle_time(i);
		cur_iowait = get_iowait_time(i);
		cur_user = get_user(i);
		cur_nice = get_nice(i);
		cur_system = get_system(i);
		cur_irq = get_irq(i);
		cur_softirq = get_softirq(i);
		cur_steal = get_steal(i);
		cur_guest = get_guest(i);
		cur_guest_nice = get_guest_nice(i);

		total_idle += cur_idle;
		total_iowait += cur_iowait;
		total_user += cur_user;
		total_nice += cur_nice;
		total_system += cur_system;
		total_irq += cur_irq;
		total_softirq += cur_softirq;
		total_steal += cur_steal;
		total_guest += cur_guest;
		total_guest_nice += cur_guest_nice;

		cur_user = nsec_to_clock_t(cur_user);
		cur_nice = nsec_to_clock_t(cur_nice);
		cur_system = nsec_to_clock_t(cur_system);
		cur_idle = nsec_to_clock_t(cur_idle);
		cur_iowait = nsec_to_clock_t(cur_iowait);
		cur_irq = nsec_to_clock_t(cur_irq);
		cur_softirq = nsec_to_clock_t(cur_softirq);
		cur_steal = nsec_to_clock_t(cur_steal);
		cur_guest = nsec_to_clock_t(cur_guest);
		cur_guest_nice = nsec_to_clock_t(cur_guest_nice);
		CPU_usage_percent[cnt] = calc_cpu_usage(cur_user, cur_nice, cur_system, cur_idle, cur_iowait, cur_irq, cur_softirq, cur_steal);
		cnt++;
	}

	total_user = nsec_to_clock_t(total_user);
	total_nice = nsec_to_clock_t(total_nice);
	total_system = nsec_to_clock_t(total_system);
	total_idle = nsec_to_clock_t(total_idle);
	total_iowait = nsec_to_clock_t(total_iowait);
	total_irq = nsec_to_clock_t(total_irq);
	total_softirq = nsec_to_clock_t(total_softirq);
	total_steal = nsec_to_clock_t(total_steal);
	total_guest = nsec_to_clock_t(total_guest);
	total_guest_nice = nsec_to_clock_t(total_guest_nice);
	total_CPU_usage_percent = calc_cpu_usage(total_user, total_nice, total_system, total_idle, total_iowait, total_irq, total_softirq, total_steal);
	
	printk(KERN_INFO "%lld", total_CPU_usage_percent);
	printk(KERN_INFO "%lld", CPU_usage_percent[0]);	
	printk(KERN_INFO "%lld", CPU_usage_percent[1]);
	printk(KERN_INFO "%lld", CPU_usage_percent[2]);	
	/*printk(KERN_INFO "%lld", total_user);
	printk(KERN_INFO "%lld", total_nice);
	printk(KERN_INFO "%lld", total_system);
	printk(KERN_INFO "%lld", total_idle);
	printk(KERN_INFO "%lld", total_iowait);
	printk(KERN_INFO "%lld", total_irq);
	printk(KERN_INFO "%lld", total_softirq);
	printk(KERN_INFO "%lld", total_steal);
	printk(KERN_INFO "%lld", total_guest);
	printk(KERN_INFO "%lld", total_guest_nice);*/
	printk(KERN_INFO "END");
}


int init_module(void){
	printk(KERN_INFO "SIS 1: start.\n");
	print_info();
	return 0;
}


void cleanup_module(void){
	printk(KERN_INFO "SIS 1: end.\n");
}
