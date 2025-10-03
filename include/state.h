#ifndef LFW_STATE
#define LFW_STATE

#include "bogon.h"

struct lfw_state {
    struct lfw_bg_tree *bg_tree;
};

extern struct lfw_state lfw_global_state;

int lfw_init_state(void);
void lfw_free_state(void);

#endif
