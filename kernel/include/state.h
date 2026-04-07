#ifndef LFW_STATE
#define LFW_STATE

#include <linux/rcupdate.h>
#include <net/net_trackers.h>

struct lfw_state {
    bool under_attack;
    struct net_device *dev;
    netdevice_tracker dev_tracker;
    struct rcu_head rcu;
};

int lfw_init_state(void);
void lfw_free_state(void);

bool lfw_state_is_under_attack(void);
void lfw_state_set_under_attack(bool new_value);

struct net_device *lfw_state_get_device(void);

#endif
