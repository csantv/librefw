#ifndef LFW_HOOKS_H
#define LFW_HOOKS_H

struct nf_hook_state;
struct sk_buff;

unsigned int ingress_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

extern struct nf_hook_ops ingress_ops;

#endif
