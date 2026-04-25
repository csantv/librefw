#ifndef LFW_STATE
#define LFW_STATE

#include <linux/types.h>

struct sk_buff;
struct genl_info;

struct lfw_state {
    bool under_attack;
    struct rcu_head rcu;
};

int lfw_init_state(void);
void lfw_free_state(void);

int lfw_init_net_state(void);
void lfw_free_net_state(void);

bool lfw_state_is_under_attack(void);
int lfw_state_set_under_attack(bool new_value);
int lfw_state_set_under_attack_nl(struct sk_buff *skb, struct genl_info *info);

#endif
