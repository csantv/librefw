#ifndef LFW_BOGON_H
#define LFW_BOGON_H

#include <linux/spinlock_types.h>
#include <linux/slab.h>

struct lfw_bg_node {
    struct lfw_bg_node *child[2];
};

struct lfw_bg_tree {
    struct kmem_cache *mem;
    rwlock_t lock;
    struct lfw_bg_node *tree;
};

struct lfw_bg_tree* lfw_init_bg_tree(void);
void lfw_free_bg_tree(struct lfw_bg_tree *state);
void lfw_free_bg_node(struct kmem_cache *mem, struct lfw_bg_node *node);
void lfw_load_bg_tree(struct lfw_bg_tree *state);
struct lfw_bg_node* lfw_create_node(struct kmem_cache *mem);

#endif
