#include "nl.h"
#include "bogon.h"
#include "hcf.h"
#include "nl_ops.h"
#include "state.h"

#include <net/genetlink.h>

// clang-format off
static struct nla_policy lfw_ip_prefix_pol[] = {
    [LFW_NLA_N_IP_ADDR] = { .type = NLA_U32 },
    [LFW_NLA_N_IP_PREFIX_LEN] = { .type = NLA_U8 }
};

static struct nla_policy hcf_ip_history_entry_pol[] = {
    [LFW_NLA_HCF_HISTORY_ENTRY_IP] = { .type = NLA_BINARY, .len = 16 },
    [LFW_NLA_HCF_HISTORY_ENTRY_HC] = { .type = NLA_U8 },
    [LFW_NLA_HCF_HISTORY_ENTRY_TTL] = { .type = NLA_U8 },
};

static struct nla_policy hcf_ip_history_pol[] = {
    [LFW_NLA_HCF_HISTORY_ENTRY] = NLA_POLICY_NESTED(hcf_ip_history_entry_pol),
};

static struct nla_policy lfw_pol[] = {
    [LFW_NLA_NUM_IP_PREFIX] = { .type = NLA_U32 },
    [LFW_NLA_IP_PREFIX] = NLA_POLICY_NESTED(lfw_ip_prefix_pol),

    [LFW_NLA_HCF_HISTORY] = NLA_POLICY_NESTED(hcf_ip_history_pol),

    [LFW_NLA_UNDER_ATTACK] = { .type = NLA_FLAG },

    [LFW_NLA_LOG_TS] = { .type = NLA_U64 },
    [LFW_NLA_LOG_LVL] = { .type = NLA_U8 },
    [LFW_NLA_LOG_MSG] = { .type = NLA_NUL_STRING },

    [LFW_NLA_HCF_IP] = { .type = NLA_U32 },
    [LFW_NLA_HCF_HC] = { .type = NLA_U8 },
    [LFW_NLA_HCF_TTL] = { .type = NLA_U8 },
};

static struct genl_ops lfw_nl_ops[] = {
    {
        .cmd = LFW_NL_CMD_SET_BOGON,
        .doit = lfw_bogon_set,
    },
    {
        .cmd = LFW_NL_CMD_SET_UNDER_ATTACK,
        .doit = lfw_state_set_under_attack_nl
    },
    {
        .cmd = LFW_NL_CMD_SET_HCF,
        .doit = hcf_register_ip_history
    }
};

static struct genl_multicast_group lfw_nl_mcgrps[] = {
    [LFW_NL_GROUP_LOG] = { .name = "log" },
    [LFW_NL_GROUP_HCF] = { .name = "hcf" },
};

static struct genl_family lfw_nl_family = {
    .name = LFW_NL_FAMILY_NAME,
    .version = LFW_NL_FAMILY_VER,
    .maxattr = LFW_NLA_MAX,
    .ops = lfw_nl_ops,
    .n_ops = ARRAY_SIZE(lfw_nl_ops),
    .mcgrps = lfw_nl_mcgrps,
    .n_mcgrps = ARRAY_SIZE(lfw_nl_mcgrps),
    .policy = lfw_pol
};
// clang-format on

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

int lfw_make_multicast_msg(u8 group, u8 cmd, void *data, lfw_nl_group_cb callback)
{
    struct sk_buff *skb = genlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb) {
        pr_err("librefw: failed to allocate memory for genl message\n");
        return -ENOMEM;
    }

    void *hdr = genlmsg_put(skb, 0, 0, &lfw_nl_family, 0, cmd);
    if (!hdr) {
        pr_err("librefw: failed to allocate memory for genl header\n");
        nlmsg_free(skb);
        return -ENOMEM;
    }

    int ret = callback(skb, data);
    if (ret)
        goto err;

    genlmsg_end(skb, hdr);

    ret = genlmsg_multicast(&lfw_nl_family, skb, 0, group, GFP_KERNEL);
    if (ret == -ESRCH) {
        return 0;
    }

    return ret;

err:
    genlmsg_cancel(skb, hdr);
    nlmsg_free(skb);
    return ret;
}
