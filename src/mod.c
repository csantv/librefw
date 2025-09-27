#include "hooks.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>

static char *net_dev_name = "eno1";
module_param(net_dev_name, charp, 0);

static int __init mod_init(void)
{
    pr_info("librefw: Registering Netfilter hooks");

    const int ret = lfw_register_hooks(net_dev_name);
    if (ret) {
        pr_err("librefw: Failed to register IPv4 ingress Netfilter hook - %d\n", ret);
        return ret;
    }

	return 0;
}

static void __exit mod_exit(void)
{
    pr_info("lfw: Unregistering Netfilter hooks");
    lfw_unregister_hooks(net_dev_name);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Toro <csantve@gmail.com>");
MODULE_DESCRIPTION("A free as in freedom firewall");
