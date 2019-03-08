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


void print_info(void) {
	int i;
	u64 idle = 0, iowait = 0, user = 0, nice = 0, system = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;
	printk(KERN_INFO "Begin");
	for_each_possible_cpu(i){
		idle += get_idle_time(i);
		iowait += get_iowait_time(i);
		user += get_user(i);
		nice += get_nice(i);
		system += get_system(i);
		irq += get_irq(i);
		softirq += get_softirq(i);
		steal += get_steal(i);
		guest += get_guest(i);
		guest_nice += get_guest_nice(i);
	}

	user = nsec_to_clock_t(user);
	nice = nsec_to_clock_t(nice);
	system = nsec_to_clock_t(system);
	idle = nsec_to_clock_t(idle);
	iowait = nsec_to_clock_t(iowait);
	irq = nsec_to_clock_t(irq);
	softirq = nsec_to_clock_t(softirq);
	steal = nsec_to_clock_t(steal);
	guest = nsec_to_clock_t(guest);
	guest_nice = nsec_to_clock_t(guest_nice);

	u64 CPU_time_since_boot, CPU_idle_time_since_boot, CPU_usage_time_since_boot, CPU_percentage; 
	CPU_time_since_boot = user + nice + system + idle + iowait + irq + softirq + steal;
	CPU_idle_time_since_boot = idle + iowait;
	CPU_usage_time_since_boot = CPU_time_since_boot - CPU_idle_time_since_boot;
	CPU_percentage = ((CPU_usage_time_since_boot * 100) / CPU_time_since_boot);

	printk(KERN_INFO "%lld", CPU_percentage);
	/*printk(KERN_INFO "%lld", user);
	printk(KERN_INFO "%lld", nice);
	printk(KERN_INFO "%lld", system);
	printk(KERN_INFO "%lld", idle);
	printk(KERN_INFO "%lld", iowait);
	printk(KERN_INFO "%lld", irq);
	printk(KERN_INFO "%lld", softirq);
	printk(KERN_INFO "%lld", steal);
	printk(KERN_INFO "%lld", guest);
	printk(KERN_INFO "%lld", guest_nice);*/
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
