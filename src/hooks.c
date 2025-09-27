#include "hooks.h"

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

unsigned int lfw_ingress_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    pr_info("Ingress packet received on device: %s\n", state->in->name);
    
    return NF_ACCEPT;
}

