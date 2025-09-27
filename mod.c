#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */

static int __init mod_init(void)
{
	printk(KERN_INFO "Hello, world\n");
	return 0;
}

static void __exit mod_exit(void)
{
	printk(KERN_INFO "Goodbye, world\n");
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL v3");
MODULE_AUTHOR("Carlos Toro <csantve@gmail.com>");
MODULE_DESCRIPTION("A free as in freedom firewall");
