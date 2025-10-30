#ifndef LFW_NL_H
#define LFW_NL_H

#define LFW_NL_FAMILY_NAME "librefw"
#define LFW_NL_MC_GRP_NAME "lfwmc"
#define LFW_NL_FAMILY_VER 1

struct sk_buff;
struct genl_info;

int lfw_nl_init(void);
void lfw_nl_destroy(void);

int lfw_nl_fn_echo(struct sk_buff *skb, struct genl_info *info);
int lfw_nl_fn_echo_mc(void);

#endif
