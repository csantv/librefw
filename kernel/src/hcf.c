#include "hcf.h"

#include <linux/spinlock.h>

// TODO: maybe use hrtimer

static struct lfw_hc_state *state = NULL;
static DEFINE_SPINLOCK(lock);

int lfw_init_hc_state(void)
{
    int ret = -ENOMEM;
    state = kzalloc(sizeof(struct lfw_hc_state), GFP_KERNEL);
    if (state == NULL) {
        return -ENOMEM;
    }
    struct lfw_hc_state *st = state;

    st->mem = kmem_cache_create("lfw_hcf_cache", sizeof(struct hcf_node), 0, SLAB_HWCACHE_ALIGN, NULL);
    if (st->mem == NULL) {
        goto err_free_state;
    }

    st->workqueue = create_workqueue("lfw_hcf_wq");
    if (!st->workqueue) {
        goto err_kmem_free;
    }

    struct hcf_node *new_tree = create_hcf_node();
    if (!new_tree) {
        goto err_destroy_wq;
    }

    rcu_assign_pointer(st->tree, new_tree);
    return 0;

err_destroy_wq:
    destroy_workqueue(st->workqueue);
    st->workqueue = NULL;
err_kmem_free:
    kmem_cache_destroy(st->mem);
    st->mem = NULL;
err_free_state:
    kfree(st);
    state = NULL;
    return ret;
}

void lfw_free_hc_state(void)
{
    if (state->workqueue) {
        destroy_workqueue(state->workqueue);
    }

    free_hcf_tree();
    kmem_cache_destroy(state->mem);
    kfree(state);
}

void free_hcf_tree(void)
{
    spin_lock(&lock);
    struct hcf_node *old_root = rcu_dereference_protected(state->tree, lockdep_is_held(&lock));
    rcu_assign_pointer(state->tree, NULL);
    spin_unlock(&lock);

    if (!old_root) {
        return;
    }

    synchronize_rcu();
    free_hcf_node(old_root);
}

void free_hcf_node(struct hcf_node *node)
{
    if (!node) {
        return;
    }

    free_hcf_node(node->child[0]);
    free_hcf_node(node->child[1]);

    kmem_cache_free(state->mem, node);
}

struct hcf_node *create_hcf_node(void)
{
    struct hcf_node *node = kmem_cache_zalloc(state->mem, GFP_KERNEL);
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

struct hcf_node *clone_hcf_node(struct hcf_node *old)
{
    struct hcf_node *new_node = kmem_cache_zalloc(state->mem, GFP_KERNEL);
    if (new_node) {
        *new_node = *old;
    }
    return new_node;
}

void lfw_do_work(struct work_struct *work)
{
    struct hcf_node *nodes_to_free[33];
    int free_count = 0, nodes_created = 0;

    struct hcf_node *new_root, *runner, *new_runner;
    struct lfw_hc_add_node_task *task = container_of(work, struct lfw_hc_add_node_task, real_work);
    spin_lock(&lock);

    // clone the whole branch until you get to a non rcu portion, then create remaining branch
    runner = rcu_dereference_protected(state->tree, lockdep_is_held(&lock));
    if (!runner) {
        spin_unlock(&lock);
        return;
    }

    new_root = clone_hcf_node(runner);
    new_runner = new_root;
    nodes_to_free[free_count++] = runner;

    // clone rcu protected path
    int remaining_bits = 31;
    for (; remaining_bits >= 8; remaining_bits--) {
        int bit = (task->source_ip >> remaining_bits) & 1;
        struct hcf_node *old_child = rcu_dereference_protected(runner->child[bit], lockdep_is_held(&lock));
        if (!old_child) {
            break;
        }
        struct hcf_node *new_child = clone_hcf_node(old_child);
        new_runner->child[bit] = new_child;
        nodes_to_free[free_count++] = old_child;

        runner = old_child;
        new_runner = new_child;
    }

    // create remaining leaf, no rcu here
    for (; remaining_bits >= 8; remaining_bits--) {
        int bit = (task->source_ip >> remaining_bits) & 1;
        new_runner->child[bit] = create_hcf_node();
        new_runner = new_runner->child[bit];
        nodes_created++;
    }

    // asign metadata
    new_runner->ttl = task->ttl;
    new_runner->hc = 64 - task->ttl;

    rcu_assign_pointer(state->tree, new_root);

    spin_unlock(&lock);
    pr_info_ratelimited("librefw: [hcf] adding new node for ip %pI4 with ttl %d, %d nodes created\n", &task->source_ip,
                        task->ttl, nodes_created);

    for (int i = 0; i < free_count; i++) {
        call_rcu(&nodes_to_free[i]->rcu, free_hcf_node_rcu);
    }

    kfree(task);
}

void free_hcf_node_rcu(struct rcu_head *rp)
{
    struct hcf_node *node = container_of(rp, struct hcf_node, rcu);
    kmem_cache_free(state->mem, node);
}

int lfw_lookup_hc_tree(u32 source_ip, u8 ttl)
{
    int result = -1;
    rcu_read_lock();
    struct hcf_node *runner = rcu_dereference(state->tree);
    if (!runner) {
        goto end;
    }

    for (int i = 31; i >= 8; i--) {
        int bit = (source_ip >> i) & 1;
        runner = rcu_dereference(runner->child[bit]);
        if (!runner) {
            goto end;
        }
    }

    int hop_count = 64 - ttl;
    if (READ_ONCE(runner->hc) == hop_count) {
        result = 1;
    } else {
        result = 0;
    }

end:
    rcu_read_unlock();
    return result;
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
