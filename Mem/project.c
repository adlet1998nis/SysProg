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
#include <linux/vmstat.h>
#include <linux/swap.h>
#include <linux/mm.h>
#include <linux/hugetlb.h>
#include <linux/mman.h>
#include <linux/mmzone.h>


#define PROCFS_MAX_SIZE	100
#define PROCFS_NAME "helper"

struct proc_dir_entry *Our_Proc_File;
char proc_buf[PROCFS_MAX_SIZE];

u64 cached, memTotal, memFree, memAvailable, buffers;    
int isNegative(u64 val){
	if(val > 0){
		return 0;
	}
	return 1;
}

u64 convertToKB(u64 val){
	u64 ans = val << (PAGE_SHIFT - 10);
	return ans;
}

int calc_mem(void){
	printk(KERN_INFO "BEGIN");
	struct sysinfo i;
	/*cached = global_node_page_state(NR_FILE_PAGES) - total_swapcache_pages() - (i -> bufferram);
	if(isNegative(cached)){
		cached = 0;
	}*/
	si_meminfo(&i);
	memTotal = i.totalram;
	memTotal = convertToKB(memTotal);
	memFree = i.freeram;
	memFree = convertToKB(memFree);
	//memAvailable = si_mem_available();
	buffers = i.bufferram; 
	buffers = convertToKB(buffers);

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
	calc_mem();
	//sprintf(buf, "MemTotal: %lld\n", memTotal);
	sprintf(buf, "MemTotal: %lld\nMemFree: %lld\nBuffers: %lld\n", memTotal, memFree, buffers);
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

