#include "bogon.h"

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

    state->tree = kmem_cache_zalloc(state->mem, GFP_KERNEL);
    if (state->tree == NULL) {
        kmem_cache_destroy(state->mem);
        kfree(state);
        return NULL;
    }

    state->lock = __RW_LOCK_UNLOCKED(state->lock);

    lfw_load_bg_tree(state);
    return state;
}

void lfw_free_bg_tree(struct lfw_bg_tree *state)
{
    if (state->mem == NULL) {
        return;
    }

    // TODO: free tree nodes
    kmem_cache_free(state->mem, state->tree);
    kmem_cache_destroy(state->mem);
    kfree(state);
}

void lfw_load_bg_tree(struct lfw_bg_tree *state)
{
    static char *test_prefixes[6] = {
        "14.102.240.0/20",
        "23.135.225.0/24",
        "23.145.53.0/24",
        "23.151.160.0/24",
        "23.154.10.0/23",
        "162.159.136.234/32"
    };

    read_lock(&state->lock);
    for (int i = 0; i < 6; ++i) {
        u8 ip_arr[4];
        const char *slash_pos = NULL;
        int ret = in4_pton(test_prefixes[i], -1, ip_arr, '/', &slash_pos);
        if (ret == 0) {
            break;
        }
        __be32 ip = *(__be32*)ip_arr;
        int prefix = 0;
        ret = kstrtoint(slash_pos + 1, 10, &prefix);
        if (ret < 0) {
            break;
        }
        pr_info("librefw: parsed ip is %pI4, prefix %d\n", &ip, prefix);

    }
    read_unlock(&state->lock);
}

