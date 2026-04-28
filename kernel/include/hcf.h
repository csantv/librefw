#pragma once

#include <linux/types.h>

struct hcf_state;
struct sk_buff;
struct genl_info;

int hcf_init_state(void);
void hcf_free_state(void);

int hcf_lookup_tree(u32 source_ip, u8 ttl);
void hcf_register_ip(u32 source_ip, u8 ttl);

int hcf_register_ip_history(struct sk_buff *skb, struct genl_info *info);
