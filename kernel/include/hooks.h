#ifndef LFW_HOOKS_H
#define LFW_HOOKS_H

#include <linux/netfilter.h>

int lfw_register_hooks(void);
void lfw_unregister_hooks(void);

unsigned int lfw_filter_ipv4_hook_fn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

#endif
