#include "hcf.h"

// TODO: maybe use hrtimer

static struct lfw_hc_state *state = NULL;

int lfw_init_hc_state(void)
{
    state = kzalloc(sizeof(struct lfw_hc_state), GFP_KERNEL);
    if (state == NULL) {
        return -ENOMEM;
    }

    state->workqueue = create_workqueue("lfw_hc_wq");
    INIT_WORK(&state->work, lfw_do_work);
    timer_setup(&state->timer, lfw_hc_timer_cb, 0);
    mod_timer(&state->timer, jiffies + msecs_to_jiffies(1000));

    return 0;
}

void lfw_free_hc_state(void)
{
    timer_shutdown_sync(&state->timer);  // replaces del_timer in older kernels
    destroy_workqueue(state->workqueue);
    kfree(state);
}

void lfw_do_work(struct work_struct *work)
{
    //struct lfw_hc_state *st = container_of(work, struct lfw_hc_state, work);
    pr_info("librefw: doing work\n");
}

// done in irq
void lfw_hc_timer_cb(struct timer_list *timer)
{
    struct lfw_hc_state *st = container_of(timer, struct lfw_hc_state, timer);
    queue_work(st->workqueue, &st->work);
    mod_timer(timer, jiffies + msecs_to_jiffies(1000));
}

