#pragma once

#include <linux/types.h>

struct hcf_state;

int hcf_init_state(void);
void hcf_free_state(void);

int hcf_lookup_tree(u32 source_ip, u8 ttl);
int hcf_register_ip(u32 source_ip, u8 ttl);
