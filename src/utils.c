#include "utils.h"

#define IPV4_BITS 32

// big endian
int in4_get_bit(u32 ip, u32 bit_index)
{
    return (ip >> (IPV4_BITS - 1 - bit_index)) & 1;
}

u32 in4_get_masked(u32 ip, int prefix_len)
{
    if (prefix_len == 0) return 0;
    uint32_t mask = (0xFFFFFFFF << (IPV4_BITS - prefix_len));
    return ip & mask;
}

