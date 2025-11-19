#include "hcf.h"

#include <linux/rwlock.h>

// TODO: maybe use hrtimer

static struct lfw_hc_state *state = NULL;
static DEFINE_RWLOCK(lock);

int lfw_init_hc_state(void)
{
    state = kzalloc(sizeof(struct lfw_hc_state), GFP_KERNEL);
    if (state == NULL) {
        return -ENOMEM;
    }
    struct lfw_hc_state *st = state;
    
    st->mem = kmem_cache_create("lfw_hc_cache", sizeof(struct lfw_hc_node), 0, SLAB_HWCACHE_ALIGN, NULL);
    if (st->mem == NULL) {
        kfree(st);
        return -ENOMEM;
    }

    st->tree = lfw_create_hc_node();
    if (st->tree == NULL) {
        kmem_cache_destroy(st->mem);
        kfree(st);
        return -ENOMEM;
    }

    state->workqueue = create_workqueue("lfw_hc_wq");
    // queue_work(st->workqueue, &st->work);
    return 0;
}

struct lfw_hc_node* lfw_create_hc_node(void)
{
    struct lfw_hc_node *node = kmem_cache_zalloc(state->mem, GFP_KERNEL);
    if (node == NULL) {
        pr_err("librefw: could not allocate memory for hc node\n");
        return NULL;
    }
    node->child[0] = NULL;
    node->child[1] = NULL;
    node->hc = 0;
    node->ttl = 0;
    return node;
}

void lfw_free_hc_node(struct lfw_hc_node *node)
{
    if (node == NULL) {
        return;
    }

    lfw_free_hc_node(node->child[0]);
    lfw_free_hc_node(node->child[1]);

    kmem_cache_free(state->mem, node);
}

void lfw_free_hc_state(void)
{
    destroy_workqueue(state->workqueue);
    lfw_free_hc_node(state->tree);
    kmem_cache_destroy(state->mem);
    kfree(state);
}

void lfw_do_work(struct work_struct *work)
{
    write_lock(&lock);
    struct lfw_hc_add_node_task *task = container_of(work, struct lfw_hc_add_node_task, real_work);
    pr_info_ratelimited("librefw: adding new node for ip %pI4 with ttl %d\n", &task->source_ip, task->ttl);
    kfree(task);
    write_unlock(&lock);
}

int lfw_add_hc_node(u32 source_ip, u8 ttl)
{
    struct lfw_hc_add_node_task *task = kzalloc(sizeof(struct lfw_hc_add_node_task), GFP_KERNEL);
    if (task == NULL) {
        pr_err_ratelimited("librefw: could not allocate memory for new hc node task\n");
        return -ENOMEM;
    }

    task->source_ip = source_ip;
    task->ttl = ttl;
    INIT_WORK(&task->real_work, lfw_do_work);

    // task was already queued (rare)
    if (!queue_work(state->workqueue, &task->real_work)) {
        pr_warn_ratelimited("librefw: add now task was already queued\n");
        kfree(task);
        return -EALREADY;
    }

    return 0;
}

