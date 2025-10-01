#include "state.h"
#include "bogon.h"

#include <linux/spinlock.h>
#include <linux/netdevice.h>

struct lfw_state lfw_global_state = {
    .bogon_tree = NULL
};

int lfw_init_state(void)
{
    int ret = 0;

    lfw_global_state.bogon_tree = lfw_init_bogon_tree_state();
    if (lfw_global_state.bogon_tree == NULL) {
        pr_warn("librefw: Could not initialize bogon tree state\n");
        ret = -EINVAL;
    }

    return ret;
}

void lfw_free_state(void)
{
    lfw_free_bogon_tree_state(lfw_global_state.bogon_tree);
}
