#include "state.h"
#include "bogon.h"
#include "hcf.h"
#include "nl.h"

#include <linux/netdevice.h>
#include <linux/rwlock.h>
#include <linux/spinlock.h>

static struct lfw_state *state = NULL;
static DEFINE_RWLOCK(lock);

int lfw_init_state(void)
{
    int ret;

    state = kzalloc(sizeof(struct lfw_state), GFP_KERNEL);
    if (!state) {
        return -ENOMEM;
    }
    state->under_attack = false;

    ret = lfw_nl_init();
    if (ret < 0) {
        goto err_free_state;
    }

    ret = lfw_init_bg_state();
    if (ret < 0) {
        pr_err("librefw: could not initialize bogon state %d\n", ret);
        goto err_nl_destroy;
    }

    ret = lfw_init_hc_state();
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
