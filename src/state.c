#include "state.h"

#include <linux/spinlock.h>
#include <linux/netdevice.h>

int lfw_init_state(char *net_dev_name)
{
    spin_lock(&lfw_global_state.lock);

    spin_unlock(&lfw_global_state.lock);

    return 0;
}

void lfw_free_state(void)
{
    spin_lock(&lfw_global_state.lock);

    spin_unlock(&lfw_global_state.lock);
}
