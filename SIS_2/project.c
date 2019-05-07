#include <linux/module.h>       
#include <linux/kernel.h>
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
#include <linux/uaccess.h>
#include <asm/types.h>

#define PROCFS_MAX_SIZE	100
#define PROCFS_NAME "helper"

struct proc_dir_entry *Our_Proc_File;
char proc_buf[PROCFS_MAX_SIZE];

u64 cur_idle[4], cur_iowait[4], cur_user[4], cur_nice[4], cur_system[4], cur_irq[4];
u64 cur_softirq[4], cur_steal[4], cur_guest[4], cur_guest_nice[4];


u64 total_idle = 0, total_iowait = 0, total_user = 0, total_nice = 0, total_system = 0, total_irq = 0;
u64 total_softirq = 0, total_steal = 0, total_guest = 0, total_guest_nice = 0;
		
u64 nsec_to_clock_t(u64 x){
#if (NSEC_PER_SEC % USER_HZ) == 0
	return div_u64(x, NSEC_PER_SEC / USER_HZ);
#elif (USER_HZ % 512) == 0
	return div_u64(x * USER_HZ / 512, NSEC_PER_SEC / 512);
#else
	return div_u64(x * 9, (9ull * NSEC_PER_SEC + (USER_HZ / 2)) / USER_HZ);
#endif
}

int calc_cpu(void){
	int i;
	u64 cnt = 0;
	printk(KERN_INFO "Begin");
	
	for_each_possible_cpu(i){
		cur_idle[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_IDLE];
		cur_iowait[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_IOWAIT];
		cur_user[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_USER];;
		cur_nice[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_NICE];
		cur_system[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
		cur_irq[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
		cur_softirq[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
		cur_steal[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
		cur_guest[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
		cur_guest_nice[cnt] = kcpustat_cpu(i).cpustat[CPUTIME_GUEST];

		total_idle += cur_idle[cnt];
		total_iowait += cur_iowait[cnt];
		total_user += cur_user[cnt];
		total_nice += cur_nice[cnt];
		total_system += cur_system[cnt];
		total_irq += cur_irq[cnt];
		total_softirq += cur_softirq[cnt];
		total_steal += cur_steal[cnt];
		total_guest += cur_guest[cnt];
		total_guest_nice += cur_guest_nice[cnt];

		cur_user[cnt] = nsec_to_clock_t(cur_user[cnt]);
		cur_nice[cnt] = nsec_to_clock_t(cur_nice[cnt]);
		cur_system[cnt] = nsec_to_clock_t(cur_system[cnt]);
		cur_idle[cnt] = nsec_to_clock_t(cur_idle[cnt]);
		cur_iowait[cnt] = nsec_to_clock_t(cur_iowait[cnt]);
		cur_irq[cnt] = nsec_to_clock_t(cur_irq[cnt]);
		cur_softirq[cnt] = nsec_to_clock_t(cur_softirq[cnt]);
		cur_steal[cnt] = nsec_to_clock_t(cur_steal[cnt]);
		cur_guest[cnt] = nsec_to_clock_t(cur_guest[cnt]);
		cur_guest_nice[cnt] = nsec_to_clock_t(cur_guest_nice[cnt]);
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
	
	printk(KERN_INFO "END");
	return 0;
}

static ssize_t procfile_read(struct file *fp, char *buf, size_t len, loff_t *off){
	static int finished = 0;
	if(finished){
		finished = 0;
		return 0;
	}
	finished = 1;
	calc_cpu();
	
	sprintf(buf, "cpu: %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld\n", 
				/*cpu0: %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld\n 
				cpu1: %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld\n
				cpu2: %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld\n",*/ 
				total_user, total_nice, total_system, total_idle, total_iowait, 
				total_irq, total_softirq, total_steal, total_guest, total_guest_nice

				/*cur_user[0], cur_nice[0], cur[0]_system, cur[0]_idle, cur[0]_iowait, 
				cur_irq[0], cur_softirq[0], cur_steal[0], cur_guest[0], cur[0]_guest_nice,
				
				cur_user[1], cur_nice[1], cur[1]_system, cur[1]_idle, cur[1]_iowait, 
				cur_irq[1], cur_softirq[1], cur_steal[1], cur_guest[1], cur[1]_guest_nice,
				
				cur_user[2], cur_nice[2], cur[2]_system, cur[2]_idle, cur[2]_iowait, 
				cur_irq[2], cur_softirq[2], cur_steal[2], cur_guest[2], cur[2]_guest_nice*/
				
				);

	return strlen(buf);
}

static struct file_operations proc_stat_operations = {
	.read = procfile_read,
};

static int start_module(void){
	Our_Proc_File = proc_create(PROCFS_NAME, 0644, NULL, &proc_stat_operations);
	return 0;
}

static void finish_module(void){
	remove_proc_entry(PROCFS_NAME, NULL);
}

module_init(start_module);
module_exit(finish_module);
