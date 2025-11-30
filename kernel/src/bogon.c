#include "bogon.h"
#include "utils.h"

#include <linux/spinlock.h>
#include <linux/inet.h>
#include <linux/kstrtox.h>
#include <linux/unaligned.h>

static struct lfw_bg_state *state = NULL;
static DEFINE_SPINLOCK(lock);

int lfw_init_bg_state(void)
{
    state = kzalloc(sizeof(struct lfw_bg_state), GFP_KERNEL);
    if (unlikely(state == NULL)) {
        return -ENOMEM;
    }
    struct lfw_bg_state *st = state;

    st->mem = kmem_cache_create("lfw_bogon_cache", sizeof(struct lfw_bg_node), 0, SLAB_HWCACHE_ALIGN, NULL);
    if (st->mem == NULL) {
        kfree(st);
        return -ENOMEM;
    }

    struct lfw_bg_tree *tree = kzalloc(sizeof(struct lfw_bg_tree), GFP_KERNEL);
    if (tree == NULL) {
        kmem_cache_destroy(st->mem);
        kfree(st);
        return -ENOMEM;
    }
    rcu_assign_pointer(st->tree, tree);

    return 0;
}

void lfw_free_bg_state(void)
{
    if (unlikely(state == NULL)) {
        return;
    }

    struct lfw_bg_state *st = state;

    call_rcu(&st->tree->rcu, lfw_free_bg_tree);
    synchronize_rcu();

    kmem_cache_destroy(st->mem);
    kfree(st);
}

void lfw_free_bg_tree(struct rcu_head *rp)
{
    struct lfw_bg_tree *tree = container_of(rp, struct lfw_bg_tree, rcu);
    lfw_free_bg_node(tree->root);
    kfree(tree);
}

struct lfw_bg_node* lfw_create_node(void)
{
    struct lfw_bg_node* node = kmem_cache_zalloc(state->mem, GFP_KERNEL);
    if (node == NULL) {
        return NULL;
    }
    node->child[0] = NULL;
    node->child[1] = NULL;
    node->is_bogon = 0;
    return node;
}

void lfw_free_bg_node(struct lfw_bg_node *node)
{
    if (node == NULL) {
        return;
    }

    lfw_free_bg_node(node->child[0]);
    lfw_free_bg_node(node->child[1]);

    kmem_cache_free(state->mem, node);
}

void lfw_load_bg_tree(struct lfw_ip_prefix *prefixes, u32 len)
{
    struct lfw_bg_tree *tree = kzalloc(sizeof(struct lfw_bg_tree), GFP_KERNEL);
    if (unlikely(tree == NULL)) {
        pr_err("librefw: could not allocate memory for new bogon tree\n");
        return;
    }
    tree->root = lfw_create_node();
    if (unlikely(tree->root == NULL)) {
        pr_err("librefw: could not allocate memory for new bogon node\n");
        return;
    }

    for (int i = 0; i < len; ++i) {
        u32 ip = get_unaligned_be32(&prefixes[i].ip_prefix);
        size_t num_nodes = 0;
        struct lfw_bg_node *runner = tree->root;

        char ip_path[32] = {};
        for (int j = 0; j < prefixes[i].ip_prefix_len; j++) {
            int bit = in4_get_bit(ip, j);
            ip_path[j] = bit + '0';
            if (runner->child[bit] == NULL) {
                runner->child[bit] = lfw_create_node();

                if (unlikely(tree->root == NULL)) {
                    pr_err("librefw: could not allocate memory for new bogon node\n");
                    return;
                }
                num_nodes++;
            }
            runner = runner->child[bit];
        }
        runner->is_bogon = 1;
    }

    pr_info("librefw: replacing old tree with new tree\n");
    spin_lock(&lock);
    struct lfw_bg_tree *old_tree = rcu_dereference_protected(state->tree, lockdep_is_held(&lock));
    rcu_assign_pointer(state->tree, tree);
    spin_unlock(&lock);

    call_rcu(&old_tree->rcu, lfw_free_bg_tree);
}

int lfw_lookup_bg_tree(u32 ip)
{
    rcu_read_lock();
    struct lfw_bg_tree *tree = rcu_dereference(state->tree);
    int result = 0;

    struct lfw_bg_node *runner = tree->root;
    if (runner == NULL) {
        result = -1;
        goto end;
    }

    for (int i = 0; i < 32; i++) {
        int bit = in4_get_bit(ip, i);

        if (runner->is_bogon) {
            result = 1;
            goto end;
        }

        runner = runner->child[bit];

        if (runner == NULL) {
            result = -2;
            goto end;
        }
    }

    if (runner != NULL && runner->is_bogon) {
        result = 1;
    }

end:
    rcu_read_unlock();
    return result;
}

