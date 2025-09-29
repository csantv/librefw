#include "state.h"
#include "hooks.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

struct lfw_state lfw_global_state = {
    .lock = __SPIN_LOCK_UNLOCKED(lfw_global_state.lock),
};

static int __init lfw_mod_init(void)
{
    pr_info("librefw: initializing\n");

    int ret = lfw_register_hooks();
    if (ret < 0) {
        return ret;
    }

    pr_info("librefw: done initializing\n");
	return 0;
}

static void __exit lfw_mod_exit(void)
{
    lfw_unregister_hooks();
    pr_info("librefw: terminated, good bye\n");
}

module_init(lfw_mod_init);
module_exit(lfw_mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Toro <csantve@gmail.com>");
MODULE_DESCRIPTION("A free as in freedom firewall");
