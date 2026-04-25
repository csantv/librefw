#include "state.h"
#include "bogon.h"
#include "hcf.h"
#include "nl.h"

#include <linux/rcupdate.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>

static struct lfw_state __rcu *state;

//static struct lfw_state *state = NULL;
static DEFINE_SPINLOCK(lock);

int lfw_init_state(void)
{
    int ret;

    struct lfw_state *st = kzalloc(sizeof(struct lfw_state), GFP_KERNEL);
    if (!st) {
        return -ENOMEM;
    }
    st->under_attack = false;
    rcu_assign_pointer(state, st); // NOLINT

    ret = lfw_nl_init();
    if (ret < 0) {
        goto err_free_state;
    }

    ret = lfw_init_bg_state();
    if (ret < 0) {
        pr_err("librefw: could not initialize bogon state %d\n", ret);
        goto err_nl_destroy;
    }

    ret = hcf_init_state();
    if (ret < 0) {
        goto err_free_bg;
    }
    return 0;

err_free_bg:
    lfw_free_bg_state();
err_nl_destroy:
    lfw_nl_destroy();
err_free_state:
    kfree(state);
    return ret;
}

void lfw_free_state(void)
{
    lfw_nl_destroy();
    lfw_free_bg_state();
    hcf_free_state();
    kfree(state);
}

bool lfw_state_is_under_attack(void)
{
    bool result;
    rcu_read_lock();
    struct lfw_state *st = rcu_dereference(state);
    result = st->under_attack;
    rcu_read_unlock();
    return result;
}

int lfw_state_set_under_attack(bool new_value)
{
    struct lfw_state *new_state, *old_state;
    new_state = kzalloc(sizeof(struct lfw_state), GFP_KERNEL);
    if (unlikely(!new_state)) {
        return -ENOMEM;
    }
    spin_lock(&lock);
    old_state = rcu_dereference_protected(state, lockdep_is_held(&lock));
    *new_state = *old_state;
    new_state->under_attack = new_value;
    rcu_assign_pointer(state, new_state);
    spin_unlock(&lock);
    kfree_rcu(old_state, rcu);
    return 0;
}
