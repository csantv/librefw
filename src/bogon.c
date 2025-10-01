#include "bogon.h"

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

