#include "hooks.h"

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>

unsigned int lfw_ingress_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (skb == NULL || state == NULL) {
        return NF_ACCEPT;
    }

    unsigned char *iph_start = skb_network_header(skb);
    if (iph_start == NULL) {
        return NF_ACCEPT;
    }

    // IP version is on the first byte
    unsigned char ip_ver = (*iph_start) >> 4;
    if (ip_ver == 4) {
        lfw_ingress_ipv4_fn(priv, skb, state);
    } else if (ip_ver == 6) {
        lfw_ingress_ipv6_fn(priv, skb, state);
    }

    return NF_ACCEPT;
}

// TODO: Implement filtering
void lfw_ingress_ipv4_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph = ip_hdr(skb);
    if (iph == NULL || iph->protocol != IPPROTO_TCP) {
        return;
    }

    struct tcphdr *tcph = tcp_hdr(skb);
    pr_info("librefw: source : %pI4:%hu | dest : %pI4:%hu | seq : %u | ack_seq : %u | window : %hu | csum : 0x%hx | urg_ptr %hu\n",
            &(iph->saddr),
            ntohs(tcph->source),
            &(iph->daddr),
            ntohs(tcph->dest),
            ntohl(tcph->seq),
            ntohl(tcph->ack_seq),
            ntohs(tcph->window),
            ntohs(tcph->check),
            ntohs(tcph->urg_ptr));
}

// TODO: Implement filtering
void lfw_ingress_ipv6_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct ipv6hdr *iph = ipv6_hdr(skb);

    if (iph == NULL || iph->nexthdr != IPPROTO_TCP) {
        return;
    }

    pr_info("librefw: source : %pI6 | dest : %pI6\n", &iph->saddr, &iph->daddr);
}

