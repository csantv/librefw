#pragma once

#include <linux/types.h>

struct sk_buff;
struct genl_info;

int lfw_nl_init(void);
void lfw_nl_destroy(void);

int lfw_bogon_set(struct sk_buff *skb, struct genl_info *info);

int lfw_nl_fn_log(u8 level, u64 timestamp, char *msg);

int lfw_log(u8 level, const char *fmt, ...);
