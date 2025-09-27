#include "state.h"
#include "hooks.h"

#include <linux/spinlock.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>

int lfw_init_state(char *net_dev_name)
{
    spin_lock(&lfw_global_state.lock);

    struct lfw_state *st = &lfw_global_state;
    st->net_dev_name = net_dev_name;
    st->net_dev = dev_get_by_name(&init_net, net_dev_name);
    if (st->net_dev == NULL) {
        return -ENODEV;
    }

    struct nf_hook_ops *ops = &st->ingress_ops;
    ops->hook = lfw_ingress_hook_func;
    ops->pf = NFPROTO_INET;
    ops->hooknum = NF_INET_INGRESS;
    ops->priority = NF_IP_PRI_FIRST;
    ops->dev = st->net_dev;

    spin_unlock(&lfw_global_state.lock);

    return 0;
}

void lfw_free_state(void)
{
    spin_lock(&lfw_global_state.lock);

    struct lfw_state *st = &lfw_global_state;
    if (st->net_dev != NULL) {
        dev_put(st->net_dev);
    }

    spin_unlock(&lfw_global_state.lock);
}
