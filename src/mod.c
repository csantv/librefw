#include "hooks.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>

static int __init mod_init(void)
{
    pr_info("Registering Netfilter hooks");

    int ret = nf_register_net_hook(&init_net, &ingress_ops);
    if (ret) {
        pr_err("Failed to register IPv4 ingress Netfilter hook: %d\n", ret);
        return ret;
    }

	return 0;
}

static void __exit mod_exit(void)
{
    pr_info("Unregistering Netfilter hooks");
    nf_unregister_net_hook(&init_net, &ingress_ops);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Toro <csantve@gmail.com>");
MODULE_DESCRIPTION("A free as in freedom firewall");
