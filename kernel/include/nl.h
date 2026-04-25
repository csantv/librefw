#pragma once

#include <linux/types.h>

struct sk_buff;
struct genl_info;

int lfw_nl_init(void);
void lfw_nl_destroy(void);

typedef int (*lfw_nl_group_cb)(struct sk_buff *skb, void *data);
int lfw_make_multicast_msg(u8 group, u8 cmd, void *data, lfw_nl_group_cb callback);

