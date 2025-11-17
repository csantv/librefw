#ifndef LFW_STATE
#define LFW_STATE

#include <linux/rcupdate.h>

struct lfw_state {
    bool under_attack;
};

extern struct lfw_state __rcu *state;

int lfw_init_state(void);
void lfw_free_state(void);

#endif
