#ifndef LFW_HOOKS_H
#define LFW_HOOKS_H

#include <linux/netfilter.h>

unsigned int lfw_ingress_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

void lfw_ingress_ipv4_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);
void lfw_ingress_ipv6_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

#endif
