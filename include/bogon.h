#ifndef LFW_BOGON_H
#define LFW_BOGON_H

#include <linux/spinlock.h>
#include <linux/slab.h>

struct lfw_bogon_node {
    struct lfw_bogon_node *child[2];
    int is_prefix;
};

struct lfw_bogon_tree_state {
    struct kmem_cache *mem;
    spinlock_t lock;
    struct lfw_bogon_node *tree;
};

struct lfw_bogon_tree_state* lfw_init_bogon_tree_state(void);
void lfw_free_bogon_tree_state(struct lfw_bogon_tree_state *state);
void lfw_load_bogon_tree(struct lfw_bogon_tree_state *state);

#endif
