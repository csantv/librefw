#include "bogon.h"
#include "utils.h"

#include <linux/inet.h>
#include <linux/kstrtox.h>
#include <linux/unaligned.h>

struct lfw_bg_tree* lfw_init_bg_tree(void)
{
    struct lfw_bg_tree *state = kzalloc(sizeof(struct lfw_bg_tree), GFP_KERNEL);
    if (state == NULL) {
        return NULL;
    }

    state->mem = kmem_cache_create("lfw_bogon_cache", sizeof(struct lfw_bg_node), 0, SLAB_HWCACHE_ALIGN, NULL);
    if (state->mem == NULL) {
        kfree(state);
        return NULL;
    }

    state->tree = lfw_create_node(state->mem);
    if (state->tree == NULL) {
        kmem_cache_destroy(state->mem);
        kfree(state);
        return NULL;
    }

    state->lock = __RW_LOCK_UNLOCKED(state->lock);

    lfw_load_bg_tree(state);
    return state;
}

struct lfw_bg_node* lfw_create_node(struct kmem_cache *mem)
{
    struct lfw_bg_node* node = kmem_cache_zalloc(mem, GFP_KERNEL);
    if (node == NULL) {
        return NULL;
    }
    node->child[0] = NULL;
    node->child[1] = NULL;
    return node;
}

void lfw_free_bg_tree(struct lfw_bg_tree *state)
{
    if (state->mem == NULL) {
        return;
    }

    write_lock(&state->lock);
    lfw_free_bg_node(state->mem, state->tree);
    write_unlock(&state->lock);

    kmem_cache_destroy(state->mem);
    kfree(state);
}

void lfw_free_bg_node(struct kmem_cache *mem, struct lfw_bg_node *node)
{
    if (node == NULL) {
        return;
    }

    lfw_free_bg_node(mem, node->child[0]);
    lfw_free_bg_node(mem, node->child[1]);

    kmem_cache_free(mem, node);
}

void lfw_load_bg_tree(struct lfw_bg_tree *state)
{
    char *test_prefixes[6] = {
        "14.102.240.0/20",
        "23.135.225.0/24",
        "23.145.53.0/24",
        "23.151.160.0/24",
        "23.154.10.0/23",
        "162.159.136.234/32"
    };

    write_lock(&state->lock);

    for (int i = 0; i < 6; ++i) {
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
        struct lfw_bg_node *runner = state->tree;
        for (int j = 0; j < prefix_len; j++) {
            int bit = in4_get_bit(ip, j);
            if (runner->child[bit] == NULL) {
                runner->child[bit] = lfw_create_node(state->mem);
                num_nodes++;
            }
            runner = runner->child[bit];
        }

        pr_info("librefw: done parsing ip %08X, %ld nodes created\n", ip, num_nodes);
    }

    write_unlock(&state->lock);
}

