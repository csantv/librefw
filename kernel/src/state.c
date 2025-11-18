#include "state.h"
#include "bogon.h"
#include "nl.h"
#include "hcf.h"

#include <linux/rwlock.h>
#include <linux/spinlock.h>

static struct lfw_state *state = NULL;
static DEFINE_RWLOCK(lock);

int lfw_init_state(void)
{
    state = kzalloc(sizeof(struct lfw_state), GFP_KERNEL);
    if (state == NULL) {
        return -ENOMEM;
    }
    state->under_attack = false;

    int ret = lfw_init_bg_state();
    if (ret < 0) {
        pr_err("librefw: could not initialize bogon state %d\n", ret);
        return ret;
    }

    ret = lfw_nl_init();
    lfw_init_hc_state();

    return ret;
}

void lfw_free_state(void)
{
    lfw_nl_destroy();
    lfw_free_bg_state();
    lfw_free_hc_state();
    kfree(state);
}

bool lfw_state_is_under_attack(void)
{
    read_lock(&lock);
    bool under_attack = state->under_attack;
    read_unlock(&lock);
    return under_attack;
}

void lfw_state_set_under_attack(bool new_value)
{
    write_lock(&lock);
    state->under_attack = new_value;
    write_unlock(&lock);
}

