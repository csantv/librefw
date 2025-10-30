#include "hooks.h"
#include "bogon.h"

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/unaligned.h>

static struct nf_hook_ops lfw_ipv4_ops = {
    .hook = lfw_filter_ipv4_hook_fn,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FILTER
};

int lfw_register_hooks(void)
{
    pr_info("librefw: Registering Netfilter hook\n");
    int ret = nf_register_net_hook(&init_net, &lfw_ipv4_ops);
    if (ret) {
        pr_err("librefw: Failed to register Netfilter hook - %d\n", ret);
    }
    return ret;
}

void lfw_unregister_hooks(void)
{
    nf_unregister_net_hook(&init_net, &lfw_ipv4_ops);
}

unsigned int lfw_filter_ipv4_hook_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph = ip_hdr(skb);
    if (unlikely(!iph)) {
        return NF_ACCEPT;
    }

    if (iph->protocol != IPPROTO_TCP) {
        return NF_ACCEPT;
    }

    if (lfw_lookup_bg_tree(get_unaligned_be32(&iph->saddr)) > 0) {
        pr_info_ratelimited("librefw: dropping packet from ip %pI4\n", &iph->saddr);
        return NF_DROP;
    }

    return NF_ACCEPT;
}

