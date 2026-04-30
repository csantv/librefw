#pragma once

#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

struct sk_buff;

struct hcf_node {
    struct hcf_node __rcu *child[2];
    u8 hc;
    u8 ttl;
    struct rcu_head rcu;
};

struct hcf_add_node_task {
    struct work_struct real_work;
    u32 source_ip;
    u8 ttl;
};

struct hcf_state {
    struct workqueue_struct *workqueue;
    struct kmem_cache *mem;
    struct hcf_node __rcu *tree;
};

struct hcf_ip_ctx {
    u32 source_ip;
    u8 ttl;
    u8 hc;
};

struct hcf_node *hcf_create_node(void);
struct hcf_node *hcf_clone_node(struct hcf_node *old);
void hcf_free_node(struct hcf_node *node);
void hcf_free_node_rcu(struct rcu_head *rp);

void hcf_swap_tree(struct hcf_node *new_tree);

void hcf_register_ip_work(struct work_struct *work);
int hcf_get_initial_ttl(u8 ttl);

int hcf_log_new_ip(u32 source_ip, u8 ttl, u8 hc);
int hcf_build_log_msg(struct sk_buff *skb, void *data);
