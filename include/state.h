#ifndef LFW_STATE
#define LFW_STATE

#include <linux/spinlock.h>
#include <linux/netfilter.h>

struct lfw_state {
    spinlock_t lock;

    char *net_dev_name;
    struct net_device *net_dev;
    struct nf_hook_ops ingress_ops;
};

extern struct lfw_state lfw_global_state;

int lfw_init_state(char *net_dev_name);
void lfw_free_state(void);

#endif
