#ifndef LFW_BOGON_H
#define LFW_BOGON_H

#include <linux/slab.h>
#include <linux/rcupdate.h>

struct lfw_bg_node {
    struct lfw_bg_node *child[2];
    int is_bogon;
};

struct lfw_bg_tree {
    struct lfw_bg_node *root;
    struct rcu_head rcu;
};

struct lfw_bg_state {
    struct kmem_cache *mem;
    struct lfw_bg_tree __rcu *tree;
};

int lfw_init_bg_state(void);
void lfw_free_bg_state(void);

void lfw_free_bg_tree(struct rcu_head *rp);

struct lfw_bg_node* lfw_create_node(void);

void lfw_free_bg_node(struct lfw_bg_node *node);
void lfw_load_bg_tree(void);
int lfw_lookup_bg_tree(u32 ip);

#endif
