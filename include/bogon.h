#ifndef LFW_BOGON_H
#define LFW_BOGON_H

#include <linux/spinlock_types.h>
#include <linux/slab.h>

struct lfw_bg_node {
    struct lfw_bg_node *child[2];
    int is_prefix;
};

struct lfw_bg_tree {
    struct kmem_cache *mem;
    rwlock_t lock;
    struct lfw_bg_node *tree;
};

struct lfw_bg_tree* lfw_init_bg_tree(void);
void lfw_free_bg_tree(struct lfw_bg_tree *state);
void lfw_load_bg_tree(struct lfw_bg_tree *state);

#endif
