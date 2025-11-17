#include "state.h"
#include "bogon.h"
#include "nl.h"

#include <linux/rcupdate.h>

static struct lfw_state __rcu *state = NULL;
DEFINE_RWLOCK(lock);

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

bool lfw_state_is_under_attack(void)
{
    rcu_read_lock();
    struct lfw_state *st = rcu_dereference(state);
    bool under_attack = st->under_attack;
    rcu_read_unlock();
    return under_attack;
}

void lfw_state_set_is_under_attack(bool new_value)
{
    struct lfw_state *new_state = kzalloc(sizeof(struct lfw_state), GFP_KERNEL);
    write_lock(&lock);
    struct lfw_state *old_state = rcu_dereference_protected(state, lockdep_is_held(&lock));
    *new_state = *old_state;
    new_state->under_attack = new_value;
    rcu_assign_pointer(state, new_state);
    write_unlock(&lock);
    kfree_rcu_mightsleep(old_state);
}


