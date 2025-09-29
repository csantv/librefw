#ifndef LFW_STATE
#define LFW_STATE

#include <linux/spinlock.h>

struct lfw_state {
    spinlock_t lock;
};

extern struct lfw_state lfw_global_state;

int lfw_init_state(void);
void lfw_free_state(void);

#endif
