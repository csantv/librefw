#include "state.h"
#include "bogon.h"
#include "nl.h"

#include <linux/spinlock.h>
#include <linux/netdevice.h>

struct lfw_state lfw_global_state = {
    .bg_tree = NULL
};

int lfw_init_state(void)
{
    int ret = 0;

    lfw_global_state.bg_tree = lfw_init_bg_tree();
    if (lfw_global_state.bg_tree == NULL) {
        pr_warn("librefw: Could not initialize bg tree state\n");
        ret = -EINVAL;
    }

    ret = lfw_nl_init();

    return ret;
}

void lfw_free_state(void)
{
    lfw_nl_destroy();
    lfw_free_bg_tree(lfw_global_state.bg_tree);
}
