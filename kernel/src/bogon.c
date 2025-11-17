#include "bogon.h"
#include "utils.h"

#include <linux/rwlock.h>
#include <linux/inet.h>
#include <linux/kstrtox.h>
#include <linux/unaligned.h>

struct lfw_bg_state *state = NULL;
DEFINE_RWLOCK(lock);

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

    lfw_load_bg_tree();

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

void lfw_load_bg_tree(void)
{
    const int num_test_prefixes = 50;
    char *test_prefixes[50] = {
        "14.102.240.0/20",
        "14.192.20.0/22",
        "23.135.225.0/24",
        "23.145.53.0/24",
        "23.151.160.0/24",
        "27.34.176.0/20",
        "27.98.192.0/20",
        "27.100.4.0/22",
        "36.37.32.0/22",
        "36.255.136.0/22",
        "42.99.116.0/22",
        "43.225.28.0/22",
        "43.228.252.0/22",
        "43.229.120.0/22",
        "43.251.228.0/22",
        "43.252.224.0/22",
        "43.255.36.0/22",
        "45.113.84.0/22",
        "45.115.140.0/22",
        "45.251.248.0/22",
        "45.252.60.0/22",
        "45.253.88.0/22",
        "45.254.232.0/22",
        "49.143.236.0/22",
        "85.217.216.0/22",
        "91.226.244.0/24",
        "100.64.0.0/10",
        "101.1.16.0/20",
        "102.192.0.0/13",
        "102.200.0.0/14",
        "102.204.0.0/21",
        "102.206.152.0/22",
        "102.206.172.0/22",
        "103.1.44.0/22",
        "103.4.205.0/24",
        "103.5.8.0/22",
        "103.38.112.0/22",
        "103.38.145.0/24",
        "110.44.160.0/21",
        "111.92.180.0/22",
        "111.92.224.0/20",
        "113.20.132.0/22",
        "113.52.232.0/21",
        "116.66.232.0/21",
        "116.204.216.0/22",
        "118.26.180.0/22",
        "118.26.184.0/22",
        "119.10.144.0/20",
        "119.15.67.0/24",
        "119.160.212.0/24",
    };

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

    for (int i = 0; i < num_test_prefixes; ++i) {
        u8 ip_arr[4];
        const char *slash_pos = NULL;
        int ret = in4_pton(test_prefixes[i], -1, ip_arr, '/', &slash_pos);
        if (ret == 0) {
            break;
        }
        u32 ip = get_unaligned_be32(ip_arr);
        int prefix_len = 0;
        ret = kstrtoint(slash_pos + 1, 10, &prefix_len);
        if (ret < 0) {
            break;
        }

        size_t num_nodes = 0;
        struct lfw_bg_node *runner = tree->root;

        char ip_path[32] = {};
        for (int j = 0; j < prefix_len; j++) {
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

        pr_info(
            "librefw: done parsing ip %s, used path %s, %ld nodes created\n", test_prefixes[i], ip_path, num_nodes);
    }

    write_lock(&lock);
    struct lfw_bg_tree *old_tree = rcu_dereference_protected(state->tree, lockdep_is_held(&lock));
    rcu_assign_pointer(state->tree, tree);
    write_unlock(&lock);

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

    char ip_path[32] = {};
    for (int i = 0; i < 32; i++) {
        int bit = in4_get_bit(ip, i);
        ip_path[i] = bit + '0';

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

