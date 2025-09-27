#ifndef LFW_HOOKS_H
#define LFW_HOOKS_H

struct nf_hook_state;
struct sk_buff;

int lfw_register_hooks(const char *net_dev_name);
int lfw_unregister_hooks(const char *net_dev_name);

unsigned int lfw_ingress_hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);

#endif
