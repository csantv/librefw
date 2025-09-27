#include "state.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>

struct lfw_state lfw_global_state = {
    .lock = __SPIN_LOCK_UNLOCKED(lfw_global_state.lock),
    .net_dev_name = NULL,
    .net_dev = NULL,
    .ingress_ops = {}
};

static char *net_dev_name = "eno1";
module_param(net_dev_name, charp, 0);

static int __init lfw_mod_init(void)
{
    pr_info("librefw: initializing, using device %s\n", net_dev_name);

    int ret = lfw_init_state(net_dev_name);
    if (ret < 0) {
        pr_err("librefw: could not initialize state, err -%d\n", ret);
        return ret;
    }

    ret = nf_register_net_hook(&init_net, &lfw_global_state.ingress_ops);
    if (ret) {
        pr_err("librefw: Failed to register ingress Netfilter hook - %d\n", ret);
        return ret;
    }

    pr_info("librefw: done initializing\n");
	return 0;
}

static void __exit lfw_mod_exit(void)
{
    pr_info("librefw: terminating\n");
    nf_unregister_net_hook(&init_net, &lfw_global_state.ingress_ops);
    lfw_free_state();
    pr_info("librefw: terminated, good bye\n");
}

module_init(lfw_mod_init);
module_exit(lfw_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Toro <csantve@gmail.com>");
MODULE_DESCRIPTION("A free as in freedom firewall");
