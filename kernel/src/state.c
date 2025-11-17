#include "state.h"
#include "bogon.h"
#include "nl.h"

struct lfw_state *state = NULL;

int lfw_init_state(void)
{
    struct lfw_state *new_state = kzalloc(sizeof(struct lfw_state), GFP_KERNEL);
    new_state->under_attack = false;
    rcu_assign_pointer(state, new_state);

    int ret = lfw_init_bg_state();
    if (ret < 0) {
        pr_err("librefw: could not initialize bogon state %d\n", ret);
        return ret;
    }

    ret = lfw_nl_init();

    return ret;
}

void lfw_free_state(void)
{
    lfw_nl_destroy();
    lfw_free_bg_state();
    kfree_rcu_mightsleep(state);
}
