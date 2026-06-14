// SPDX-License-Identifier: GPL-2.0-only
/*
 * librefw: a free as in freedom firewall
 * 
 * Copyright (C) 2026 Carlos Santos Toro Vera
 */

#pragma once

#include <linux/types.h>

struct sk_buff;
struct genl_info;

struct lfw_ip_prefix {
    u32 ip_prefix;
    u8 ip_prefix_len;
};

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
void lfw_load_bg_tree(struct lfw_ip_prefix *prefixes, u32 len);
int lfw_lookup_bg_tree(u32 ip);

int lfw_bogon_set(struct sk_buff *skb, struct genl_info *info);
