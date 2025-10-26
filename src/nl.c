#include "nl.h"

#include <net/genetlink.h>

enum lfw_nl_attrs {
    LFW_NL_A_UNSPEC,
    LFW_NL_A_MSG,
    __LFW_NL_A_MAX,
};

#define LFW_NL_A_MAX (__LFW_NL_A_MAX - 1)

enum lfw_nl_cmds {
    LFW_NL_CMD_UNSPEC,
    LFW_NL_CMD_ECHO,
    __LFW_NL_CMD_MAX,
};

#define LFW_NL_CMD_MAX (__LFW_NL_CMD_MAX - 1)

static struct genl_ops lfw_nl_ops[] = {
};

static struct genl_family lfw_nl_family = {
    .name = LFW_NL_FAMILY_NAME,
    .version = LFW_NL_FAMILY_VER,
    .netnsok = false,
    .module = THIS_MODULE,
    .ops = lfw_nl_ops
};

int lfw_nl_init(void)
{
    int ret = genl_register_family(&lfw_nl_family);

    if (ret) {
        pr_err("librefw: Failed to register family: %d\n", ret);
        return ret;
    }

    pr_info("librefw: Initialized Netlink family\n");
    return 0;
}

void lfw_nl_destroy(void)
{
    genl_unregister_family(&lfw_nl_family);
    pr_info("librefw: Removing nl server\n");
}
