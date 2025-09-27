#include "hooks.h"

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

unsigned int lfw_ingress_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    pr_info("Ingress packet received on device: %s\n", state->in->name);
    
    return NF_ACCEPT;
}

int lfw_register_hooks(const char *net_dev_name)
{
    struct net_device *net_dev = dev_get_by_name(&init_net, net_dev_name);
    if (net_dev == NULL) {
        return -ENODEV;
    }
    const struct nf_hook_ops ingress_ops = {
        .hook = lfw_ingress_hook_func,
        .pf = NFPROTO_INET,  // handle ipv4 and ivp6
        .hooknum = NF_INET_INGRESS,
        .priority = NF_IP_PRI_FIRST,
        .dev = net_dev
    };
    const int res = nf_register_net_hook(&init_net, &ingress_ops);
    dev_put(net_dev);
    return res;
}

int lfw_unregister_hooks(const char *net_dev_name)
{
    struct net_device *net_dev = dev_get_by_name(&init_net, net_dev_name);
    if (net_dev == NULL) {
        return -ENODEV;
    }
    const struct nf_hook_ops ingress_ops = {
        .hook = lfw_ingress_hook_func,
        .pf = NFPROTO_INET,  // handle ipv4 and ivp6
        .hooknum = NF_INET_INGRESS,
        .priority = NF_IP_PRI_FIRST,
        .dev = net_dev
    };
    nf_unregister_net_hook(&init_net, &ingress_ops);
    return 0;
}