#include "state.h"
#include "bogon.h"

#include <linux/spinlock.h>
#include <linux/netdevice.h>

int lfw_init_state(void)
{
    int ret = 0;

    spin_lock(&lfw_global_state.lock);

    lfw_global_state.bogon_tree = lfw_init_bogon_tree_state();
    if (lfw_global_state.bogon_tree == NULL) {
        pr_warn("librefw: Could not initialize bogon tree state\n");
        ret = -EINVAL;
    }

    spin_unlock(&lfw_global_state.lock);

    return ret;
}

void lfw_free_state(void)
{
    spin_lock(&lfw_global_state.lock);

    lfw_free_bogon_tree_state(lfw_global_state.bogon_tree);

    spin_unlock(&lfw_global_state.lock);
}
