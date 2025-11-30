#include "nl.h"
#include "nl_ops.h"

#include <net/genetlink.h>

static struct nla_policy lfw_ip_prefix_pol[] = {
    [LFW_NL_A_N_IP_ADDR] = { .type = NLA_BE32 },
    [LFW_NL_A_N_IP_PREFIX_LEN] = { .type = NLA_U8 }
};

static struct nla_policy lfw_pol[] = {
    [LFW_NL_A_MSG] = { .type = NLA_NUL_STRING },
    [LFW_NL_A_NUM_IP_PREFIX] = { .type = NLA_U32 },
    [LFW_NL_A_IP_PREFIX] = NLA_POLICY_NESTED(lfw_ip_prefix_pol)
};

static struct genl_ops lfw_nl_ops[] = {
    {
        .cmd = LFW_NL_CMD_ECHO,
        .doit = lfw_nl_fn_echo
    },
    {
        .cmd = LFW_NL_CMD_BOGON_SET,
        .doit = lfw_bogon_set
    }
};

static struct genl_multicast_group lfw_nl_mcgrps[] = {
    { .name = LFW_NL_MC_GRP_NAME }
};

static struct genl_family lfw_nl_family = {
    .name = LFW_NL_FAMILY_NAME,
    .version = LFW_NL_FAMILY_VER,
    .maxattr = LFW_NL_A_MAX,
    .ops = lfw_nl_ops,
    .n_ops = ARRAY_SIZE(lfw_nl_ops),
    .mcgrps = lfw_nl_mcgrps,
    .n_mcgrps = ARRAY_SIZE(lfw_nl_mcgrps),
    .policy = lfw_pol
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

int lfw_nl_fn_echo(struct sk_buff *skb, struct genl_info *info)
{
    if (info->attrs[LFW_NL_A_MSG]) {
        char *str = nla_data(info->attrs[LFW_NL_A_MSG]);
        pr_info("librefw: message received: %s\n", str);
    } else {
        pr_info("librefw: empty message received\n");
    }

    struct sk_buff *msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!msg) {
        pr_err("librefw: failed to allocate message buffer\n");
        return -ENOMEM;
    }

    void *hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq, &lfw_nl_family, 0, LFW_NL_CMD_ECHO);
    if (!hdr) {
        pr_err("librefw: failed to create genetlink header\n");
        nlmsg_free(msg);
        return -EMSGSIZE;
    }

    int ret = nla_put_string(msg, LFW_NL_A_MSG, "Hello from Netlink!");
    if (ret) {
        pr_err("failed to create message string\n");
        genlmsg_cancel(msg, hdr);
        nlmsg_free(msg);
        goto out;
    }

    genlmsg_end(msg, hdr);

    ret = genlmsg_reply(msg, info);
    pr_info("librefw: reply sent\n");

out:
    return ret;
}

int lfw_bogon_set(struct sk_buff *skb, struct genl_info *info)
{
    pr_info("librefw: received bogon list\n");

    u32 num_prefix = nla_get_u32_default(info->attrs[LFW_NL_A_NUM_IP_PREFIX], 0);
    pr_info("librefw: get number of prefixes %d\n", num_prefix);

    struct nlattr *pos = NULL;
    int rem = 0;
    nla_for_each_attr_type(pos, LFW_NL_A_IP_PREFIX, nlmsg_attrdata(info->nlhdr, GENL_HDRLEN), nlmsg_attrlen(info->nlhdr, GENL_HDRLEN), rem) {
        int nested_rem = 0;
        struct nlattr *nested_pos = NULL;
        nla_for_each_nested(nested_pos, pos, nested_rem) {
            switch (nla_type(nested_pos)) {
                case LFW_NL_A_N_IP_ADDR:
                    pr_info("librefw: got ip addr %d\n", nla_get_be32(nested_pos));
                    break;
                case LFW_NL_A_N_IP_PREFIX_LEN:
                    pr_info("librefw: got prefix len %d\n", nla_get_u8(nested_pos));
                    break;
                default:
                    pr_warn("librefw: got unexpected value in bogon message\n");
            }
        }
    }

    return 0;
}

int lfw_nl_fn_echo_mc(void)
{
    struct sk_buff *skb = genlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb) {
        pr_err("librefw: failed to allocate memory for genl message\n");
        return -ENOMEM;
    }

    void *hdr = genlmsg_put(skb, 0, 0, &lfw_nl_family, 0, LFW_NL_CMD_ECHO);
    if (!hdr) {
        pr_err("librefw: failed to allocate memory for genl header\n");
        nlmsg_free(skb);
        return -ENOMEM;
    }

    int ret = nla_put_string(skb, LFW_NL_A_MSG, "Sending notification");
    if (ret) {
        pr_err("librefw: unable to send multicast message\n");
        genlmsg_cancel(skb, hdr);
        nlmsg_free(skb);
        return ret;
    }

    genlmsg_end(skb, hdr);

    ret = genlmsg_multicast(&lfw_nl_family, skb, 0, 0, GFP_KERNEL);
    if (ret == -ESRCH) {
        pr_warn("librefw: multicast message sent, but nobody was listening...\n");
    } else if (ret) {
        pr_err("librefw: failed to send multicast genl message\n");
    } else {
        pr_info("librefw: multicast mssage sent\n");
    }

    return ret;
}
