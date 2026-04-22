#ifndef LFW_STATE
#define LFW_STATE

#include <linux/rcupdate.h>

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

#endif
