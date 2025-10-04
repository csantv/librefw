#ifndef LFW_UTILS
#define LFW_UTILS

#include <linux/types.h>

int in4_get_bit(u32 ip, u32 bit_index);
u32 in4_get_masked(u32 ip, int prefix_len);

#endif
