#pragma once

#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/timer.h>

struct lfw_hc_state {
    struct work_struct work;
    struct workqueue_struct *workqueue;
    struct timer_list timer;
};

int lfw_init_hc_state(void);
void lfw_free_hc_state(void);

void lfw_do_work(struct work_struct *work);
void lfw_hc_timer_cb(struct timer_list *timer);
