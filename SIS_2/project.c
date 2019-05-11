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
#include <linux/swap_slots.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#define PROCFS_MAX_SIZE	100
#define PROCFS_NAME "helper"
MODULE_LICENSE("GPL");

struct proc_dir_entry *Our_Proc_File;
char proc_buf[PROCFS_MAX_SIZE];
unsigned long pages[NR_LRU_LISTS];

u64 cached, memTotal, memFree, buffers, active, inactive;
char cur[100];
char cur1[100];
char cur2[100];
char cur3[100];
char cur4[100];
char cur5[100];
char mem_ans[1001], cpu_ans[1001], user_ans[1001], uptime_ans[101];

char cur6[100];
char cur7[100];
char cur8[100];
char cur9[100];
char cur10[100];
char cur11[100];
char cur12[100];
char cur13[100];
char cur14[100];
char cur15[100];

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

void toString(char c[], int num){
    u64 i, size = 0;
 	u64 val = num;
    while (val > 0){
        size++;
        val /= 10;
    }

    if(size == 0){
    	c[0] = '0';
    	return ;
    }
    
    for (i = 0; i < size; i++){
        c[size - (i + 1)] = (num % 10) + '0';
        num /= 10;
    }
}

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

void calc_pages(void){
	int j;
	for(j = LRU_BASE; j < NR_LRU_LISTS; j++){
		pages[j] = global_node_page_state(j + NR_LRU_BASE);
	}
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
	
	toString(cur6, total_user);
	toString(cur7, total_nice);
	toString(cur8, total_system);
	toString(cur9, total_idle);
	toString(cur10, total_iowait);
	toString(cur11, total_irq);
	toString(cur12, total_softirq);
	toString(cur13, total_steal);
	toString(cur14, total_guest);
	toString(cur15, total_guest_nice);
	cpu_ans[0] = '\0';
	strcat(cpu_ans, "user:");
	strcat(cpu_ans, cur6);
	strcat(cpu_ans, "\n");

	strcat(cpu_ans, "nice:");
	strcat(cpu_ans, cur7);
	strcat(cpu_ans, "\n");

	strcat(cpu_ans, "system:");
	strcat(cpu_ans, cur8);
	strcat(cpu_ans, "\n");

	strcat(cpu_ans, "idle:");
	strcat(cpu_ans, cur9);
	strcat(cpu_ans, "\n");

	strcat(cpu_ans, "iowait:");
	strcat(cpu_ans, cur10);
	strcat(cpu_ans, "\n");
	
	strcat(cpu_ans, "irq:");
	strcat(cpu_ans, cur11);
	strcat(cpu_ans, "\n");

	strcat(cpu_ans, "softirq:");
	strcat(cpu_ans, cur12);
	strcat(cpu_ans, "\n");

	strcat(cpu_ans, "steal:");
	strcat(cpu_ans, cur13);
	strcat(cpu_ans, "\n");

	strcat(cpu_ans, "guest:");
	strcat(cpu_ans, cur14);
	strcat(cpu_ans, "\n");

	strcat(cpu_ans, "guest_nice:");
	strcat(cpu_ans, cur15);
	strcat(cpu_ans, "\n");
	
	printk(KERN_INFO "END");
	return 0;
}

int calc_mem(void){
	printk(KERN_INFO "BEGIN");
	struct sysinfo i;
	calc_pages();
	si_meminfo(&i);
	memTotal = i.totalram;
	memTotal = convertToKB(memTotal);
	toString(cur, memTotal);
	
	memFree = i.freeram;
	memFree = convertToKB(memFree);
	toString(cur1, memFree);
	
	cached = global_node_page_state(NR_FILE_PAGES) - i.bufferram;
	cached = convertToKB(cached);
	if(isNegative(cached)){
		cached = 0;
	}
	toString(cur2, cached);
	buffers = i.bufferram; 
	buffers = convertToKB(buffers);
	toString(cur3, buffers);
	
	active = pages[LRU_ACTIVE_ANON] + pages[LRU_ACTIVE_FILE];
	active = convertToKB(active);
	toString(cur4, active);
	
	inactive = pages[LRU_INACTIVE_ANON] + pages[LRU_INACTIVE_FILE];
	inactive = convertToKB(inactive);
	toString(cur5, inactive);
	mem_ans[0] = '\0';
	strcat(mem_ans, "MemTotal:");
	strcat(mem_ans, cur);
	strcat(mem_ans, "\n");
	
	strcat(mem_ans, "MemFree:");
	strcat(mem_ans, cur1);
	strcat(mem_ans, "\n");

	strcat(mem_ans, "Cached:");
	strcat(mem_ans, cur2);
	strcat(mem_ans, "\n");


	strcat(mem_ans, "Buffers:");
	strcat(mem_ans, cur3);
	strcat(mem_ans, "\n");

	strcat(mem_ans, "Active:");
	strcat(mem_ans, cur4);
	strcat(mem_ans, "\n");

	strcat(mem_ans, "Inactive:");
	strcat(mem_ans, cur5);
	strcat(mem_ans, "\n");

	printk(KERN_INFO "END");
	return 0;
}

struct file* file_open(const char* path, int flags, int rights) 
{
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

int file_read(struct file *file) {
    mm_segment_t oldfs;

    oldfs = get_fs();
    set_fs(get_ds());
    int USERNAME_SIZE = 50;

    file->f_pos=0;    

    int i;

    for (i=0;i<100; i++) {
        char username[USERNAME_SIZE];
        int a;
        for (a=0;a <USERNAME_SIZE-1; a++) username[a] = '\0';
        
        char buff[1];
        buff[0] = 10;

        a = 0;
        int ret = vfs_read(file,buff,1,&file->f_pos); 
        
        if (!ret) {
            break;
        }

        while(buff[0] != ':') {
            username[a++] = buff[0];
            vfs_read(file,buff,1,&file->f_pos);
        }

        username[a] = '\0';
        while(buff[0] != 10) { 
            vfs_read(file,buff,1,&file->f_pos);
        }

        strcat(user_ans, username);
        strcat(user_ans, "\n");
    }

    set_fs(oldfs);
    return 0;
}  

void get_users(void) {
	printk(KERN_INFO "BEGIN");

    struct file* file = file_open("/etc/passwd", O_RDONLY, 0);
    if (file != NULL) {
    	user_ans[0] = '\0';
    	strcat(user_ans, "Users:\n");
    	file_read(file); 
    }

	printk(KERN_INFO "END");
}


void calc_uptime(void) {
	struct timespec  uptime;
	get_monotonic_boottime(&uptime);
	unsigned long uptime_in_seconds = uptime.tv_sec;

	int seconds = uptime_in_seconds % 60;
	int minutes = uptime_in_seconds / 60 % 60;
	int hours   = uptime_in_seconds / 3600 % 24;
	int days    = uptime_in_seconds / 86400;

	char daystring[32] = "";
   	if (days > 1) {
      sprintf(daystring, "%d days, ", days);
   	} else if (days == 1) {
      sprintf(daystring, "%d days, ", days);
   	}

 	sprintf(uptime_ans, "Uptime:\n%s%02d:%02d:%02d", daystring, hours, minutes, seconds);
}

static ssize_t procfile_read(struct file *fp, char *buf, size_t len, loff_t *off){
	static int finished = 0;
	if(finished){
		finished = 0;
		return 0;
	}
	finished = 1;
	calc_mem();
	calc_cpu();
	get_users();
	calc_uptime();

	sprintf(buf, "%s\n\n%s\n\n%s\n\n%s\n\n", cpu_ans, mem_ans, user_ans, uptime_ans);
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