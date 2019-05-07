#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h> 
 
static int irq=10;
module_param(irq,int,0660);
 
static int simple_init(void)
{
	printk(KERN_WARNING "hello... irq=%d name=%s debug=%d\n",irq,devname,debug);
	return 0;
}
 
static void simple_cleanup(void)
{
	printk(KERN_WARNING "bye... irq=%d name=%s debug=%d\n",irq,devname,debug);
}
 
module_init(simple_init);
module_exit(simple_cleanup);
