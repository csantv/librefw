#include "bogon.h"

#include <linux/inet.h>

struct lfw_bogon_tree_state* lfw_init_bogon_tree_state(void)
{
    struct lfw_bogon_tree_state *state = kzalloc(sizeof(struct lfw_bogon_tree_state), GFP_KERNEL);
    if (state == NULL) {
        return NULL;
    }

    state->mem = kmem_cache_create("lfw_bogon_cache", sizeof(struct lfw_bogon_node), 0, SLAB_HWCACHE_ALIGN, NULL);
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

    state->lock = __SPIN_LOCK_UNLOCKED(state->lock);

    return state;
}

void lfw_free_bogon_tree_state(struct lfw_bogon_tree_state *state)
{
    if (state->mem == NULL) {
        return;
    }

    // TODO: free tree nodes
    kmem_cache_free(state->mem, state->tree);
    kmem_cache_destroy(state->mem);
    kfree(state);
}

void lfw_load_bogon_tree(struct lfw_bogon_tree_state *state)
{
    char *test_prefixes[6] = {
        "14.102.240.0/20",
        "23.135.225.0/24",
        "23.145.53.0/24",
        "23.151.160.0/24",
        "23.154.10.0/23",
        "162.159.136.234/32"
    };

    spin_lock(&state->lock);
    spin_unlock(&state->lock);
}

