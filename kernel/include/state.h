#ifndef LFW_STATE
#define LFW_STATE

#include <linux/types.h>

struct lfw_state {
    bool under_attack;
};

int lfw_init_state(void);
void lfw_free_state(void);

bool lfw_state_is_under_attack(void);
void lfw_state_set_is_under_attack(bool new_value);

#endif
