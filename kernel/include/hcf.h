#pragma once

#include <linux/workqueue.h>
#include <linux/slab.h>

struct lfw_hc_node {
    struct lfw_hc_node *child[2];
    u8 hc;
    u8 ttl;
};

struct lfw_hc_add_node_task {
    struct work_struct real_work;
    u32 source_ip;
    u8 ttl;
};

struct lfw_hc_state {
    struct workqueue_struct *workqueue;
    struct kmem_cache *mem;
    struct lfw_hc_node *tree;
};

int lfw_init_hc_state(void);
void lfw_free_hc_state(void);

void lfw_do_work(struct work_struct *work);
int lfw_lookup_hc_tree(u32 source_ip, u8 tll);

struct lfw_hc_node* lfw_create_hc_node(void);
void lfw_free_hc_node(struct lfw_hc_node *node);
int lfw_add_hc_node(u32 source_ip, u8 ttl);
