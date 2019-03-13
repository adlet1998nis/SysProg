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

#define PROCFS_MAX_SIZE	30
#define PROCFS_NAME "sis1"

u64 CPU_usage_percent[3];
u64 total_CPU_usage_percent = 0;

struct proc_dir_entry *Our_Proc_File;
char proc_buf[PROCFS_MAX_SIZE];

/*static ssize_t procfile_write(struct file *fp, const char *buf, size_t len, loff_t *off){
	if(len > PROCFS_MAX_SIZE){
		return -EFAULT;
	}

	if(copy_from_user(proc_buf, buf, len)){
		return -EFAULT;
	}

	total_CPU_usage_percent = simple_strtoul(proc_buf, NULL, 10);
	CPU_usage_percent[0] = simple_strtoul(proc_buf, NULL, 10);
	CPU_usage_percent[1] = simple_strtoul(proc_buf, NULL, 10);
	CPU_usage_percent[2] = simple_strtoul(proc_buf, NULL, 10);
	
	return len;
}*/

static ssize_t procfile_read(struct file *fp, char *buf, size_t len, loff_t *off){
	static int finished = 0;
	if(finished){
		finished = 0;
		return 0;
	}
	finished = 1;
	sprintf(buf, "cpu:%lld\ncpu0:%lld\ncpu1:%lld\ncpu2:%lld\n", total_CPU_usage_percent, CPU_usage_percent[0], CPU_usage_percent[1], CPU_usage_percent[2]);
	return strlen(buf);
}

static struct file_operations proc_stat_operations = {
	//.write		= procfile_write,
	.read		= procfile_read,
};

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

int calc_info(void){
	int i;
	u64 cnt = 0;
	u64 total_idle = 0, total_iowait = 0, total_user = 0, total_nice = 0, total_system = 0, total_irq = 0, total_softirq = 0, total_steal = 0, total_guest = 0, total_guest_nice = 0;
	u64 cur_idle = 0, cur_iowait = 0, cur_user = 0, cur_nice = 0, cur_system = 0, cur_irq = 0, cur_softirq = 0, cur_steal = 0, cur_guest = 0, cur_guest_nice = 0;
	printk(KERN_INFO "Begin");
	for_each_possible_cpu(i){
		cur_idle = kcpustat_cpu(i).cpustat[CPUTIME_IDLE];
		cur_iowait = kcpustat_cpu(i).cpustat[CPUTIME_IOWAIT];
		cur_user = kcpustat_cpu(i).cpustat[CPUTIME_USER];;
		cur_nice = kcpustat_cpu(i).cpustat[CPUTIME_NICE];
		cur_system = kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
		cur_irq = kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
		cur_softirq = kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
		cur_steal = kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
		cur_guest = kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
		cur_guest_nice = kcpustat_cpu(i).cpustat[CPUTIME_GUEST];

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
	
	/*printk(KERN_INFO "%lld", total_CPU_usage_percent);
	printk(KERN_INFO "%lld", CPU_usage_percent[0]);	
	printk(KERN_INFO "%lld", CPU_usage_percent[1]);
	printk(KERN_INFO "%lld", CPU_usage_percent[2]);	
	printk(KERN_INFO "%lld", total_user);
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
	return 0;
}

static int start_module(void){
	calc_info();
	Our_Proc_File = proc_create(PROCFS_NAME, 0644, NULL, &proc_stat_operations);
	return 0;
}

static void finish_module(void){
	remove_proc_entry(PROCFS_NAME, NULL);
}

module_init(start_module);
module_exit(finish_module);
