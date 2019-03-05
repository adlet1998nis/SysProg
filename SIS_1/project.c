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


void print_info(void) {
	int i;
	u64 idle = 0;
	u64 iowait = 0;
	printk(KERN_INFO "Begin");
	for_each_possible_cpu(i){
		idle += get_idle_time(i);
		iowait += get_iowait_time(i);
	}

	printk(KERN_INFO "%lld", idle);
	printk(KERN_INFO "%lld", iowait);
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
