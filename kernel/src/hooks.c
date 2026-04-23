#include "hooks.h"
#include "bogon.h"
#include "hcf.h"
#include "state.h"

#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/tcp.h>
#include <linux/unaligned.h>

struct lfw_net_state {
    struct net_device *dev;
    netdevice_tracker dev_tracker;
};

static struct lfw_net_state net;

static struct nf_hook_ops lfw_ipv4_ops[2] = {
    {.hook = lfw_filter_ipv4_hook_fn,
     .pf = NFPROTO_NETDEV,
     .hooknum = NF_NETDEV_INGRESS,
     .priority = NF_IP_PRI_FIRST,
     .dev = NULL},
    {.hook = lfw_hc_learn_ipv4_hook_fn, .pf = NFPROTO_IPV4, .hooknum = NF_INET_LOCAL_IN, .priority = NF_IP_PRI_LAST}};

int lfw_register_hooks(void)
{
    net.dev = netdev_get_by_name(&init_net, "eno1", &net.dev_tracker, GFP_KERNEL);
    if (!net.dev) {
        pr_err("librefw: could not get device eno1\n");
        return -ENODEV;
    }

    lfw_ipv4_ops[0].dev = net.dev;
    pr_info("librefw: Registering Netfilter hook on device eno1\n");
    int ret = nf_register_net_hooks(&init_net, lfw_ipv4_ops, 2);
    if (ret) {
        pr_err("librefw: Failed to register Netfilter hook - %d\n", ret);
        netdev_put(net.dev, &net.dev_tracker);
        net.dev = NULL;
        return ret;
    }

    return 0;
}

void lfw_unregister_hooks(void)
{
    if (!net.dev) {
        return;
    }
    nf_unregister_net_hooks(&init_net, lfw_ipv4_ops, 2);
    netdev_put(net.dev, &net.dev_tracker);
}

unsigned int lfw_filter_ipv4_hook_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct ethhdr *eth = eth_hdr(skb);
    struct iphdr _iph, *iph;

    if (unlikely(!eth || ntohs(eth->h_proto) != ETH_P_IP)) {
        return NF_ACCEPT;
    }

    iph = skb_header_pointer(skb, skb_network_offset(skb), sizeof(_iph), &_iph);
    if (!iph || iph->protocol != IPPROTO_TCP) {
        return NF_ACCEPT;
    }

    __be32 saddr = get_unaligned_be32(&iph->saddr);
    if (lfw_lookup_bg_tree(saddr) > 0) {
        pr_info_ratelimited("librefw: dropping packet from ip %pI4\n", &iph->saddr);
        return NF_DROP;
    }

    if (lfw_state_is_under_attack() && lfw_lookup_hc_tree(saddr, iph->ttl) < 1) {
        pr_info_ratelimited("librefw: dropping packet from ip %pI4\n", &iph->saddr);
        return NF_DROP;
    }

    return NF_ACCEPT;
}

unsigned int lfw_hc_learn_ipv4_hook_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph = ip_hdr(skb);
    if (unlikely(!iph)) {
        return NF_ACCEPT;
    }

    if (iph->protocol != IPPROTO_TCP && iph->protocol != IPPROTO_UDP) {
        return NF_ACCEPT;
    }

    // stop learning if under attack
    if (lfw_state_is_under_attack()) {
        return NF_ACCEPT;
    }

    int ret = lfw_add_hc_node(get_unaligned_be32(&iph->saddr), iph->ttl);
    if (unlikely(ret < 0)) {
        pr_warn_ratelimited("librefw: lfw_add_hc_node - %d\n", ret);
    }

    return NF_ACCEPT;
}
