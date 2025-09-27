#include "hooks.h"

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

unsigned int ingress_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    pr_info("Ingress packet received on device: %s\n", state->in->name);
    
    return NF_ACCEPT;
}

struct nf_hook_ops ingress_ops = {
    .hook = ingress_hook_func,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_INGRESS,
    .priority = NF_IP_PRI_FIRST
};

