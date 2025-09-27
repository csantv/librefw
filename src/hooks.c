#include "hooks.h"

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>

unsigned int lfw_ingress_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    if (skb == NULL) {
        pr_info("librefw: sk_buff is NULL for some reason\n");
        return NF_ACCEPT;
    }

    struct iphdr * iph = ip_hdr(skb);

    if (iph != NULL && iph->protocol == IPPROTO_TCP) {
        struct tcphdr *tcph = tcp_hdr(skb);
        if (tcph != NULL) {
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

        // TODO: Use radix trees for filtering IPs
    }

    return NF_ACCEPT;
}

