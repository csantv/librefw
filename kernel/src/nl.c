#include "nl.h"
#include "nl_ops.h"
#include "bogon.h"

#include <net/genetlink.h>
#include <linux/vmalloc.h>
#include <linux/ktime.h>

static struct nla_policy lfw_ip_prefix_pol[] = {
    [LFW_NLA_N_IP_ADDR] = { .type = NLA_U32 },
    [LFW_NLA_N_IP_PREFIX_LEN] = { .type = NLA_U8 }
};

static struct nla_policy lfw_pol[] = {
    [LFW_NLA_NUM_IP_PREFIX] = { .type = NLA_U32 },
    [LFW_NLA_IP_PREFIX] = NLA_POLICY_NESTED(lfw_ip_prefix_pol),

    [LFW_NLA_LOG_TS] = { .type = NLA_U64 },
    [LFW_NLA_LOG_LVL] = { .type = NLA_U8 },
    [LFW_NLA_LOG_MSG] = { .type = NLA_NUL_STRING }
};

static struct genl_ops lfw_nl_ops[] = {
    {
        .cmd = LFW_NLX_BOGON_SET,
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

int lfw_bogon_set(struct sk_buff *skb, struct genl_info *info)
{
    u32 num_prefix = nla_get_u32_default(info->attrs[LFW_NLA_NUM_IP_PREFIX], 0);
    if (!num_prefix) {
        pr_warn("librefw: did not receive number of prefixes\n");
        return -EINVAL;
    }

    pr_info("librefw: inserting %d prefixes into bogon filter list\n", num_prefix);
    struct lfw_ip_prefix *prefix_buf = vzalloc(sizeof(struct lfw_ip_prefix) * num_prefix);
    if (prefix_buf == NULL) {
        pr_warn("librefw: could not allocate buffer for ip prefixes\n");
        return -ENOMEM;
    }
    struct lfw_ip_prefix *runner = prefix_buf;

    struct nlattr *pos = NULL;
    int rem = 0;
    nla_for_each_attr_type(pos, LFW_NLA_IP_PREFIX, nlmsg_attrdata(info->nlhdr, GENL_HDRLEN), nlmsg_attrlen(info->nlhdr, GENL_HDRLEN), rem) {
        int nested_rem = 0;
        struct nlattr *nested_pos = NULL;
        nla_for_each_nested(nested_pos, pos, nested_rem) {
            switch (nla_type(nested_pos)) {
                case LFW_NLA_N_IP_ADDR:
                    runner->ip_prefix = nla_get_u32(nested_pos);
                    break;
                case LFW_NLA_N_IP_PREFIX_LEN:
                    runner->ip_prefix_len = nla_get_u8(nested_pos);
                    break;
                default:
                    pr_warn("librefw: got unexpected value in bogon message\n");
            }
        }
        runner++;
    }

    lfw_load_bg_tree(prefix_buf, num_prefix);
    vfree(prefix_buf);

    lfw_log(LOGLEVEL_INFO, "done inserting %d prefixes into bogon filter list\n", num_prefix);
    pr_info("librefw: done inserting %d prefixes into bogon filter list\n", num_prefix);

    return 0;
}

int lfw_log(u8 level, const char *fmt, ...)
{
    struct timespec64 ts;
    ktime_get_ts64(&ts);

    va_list args;
    va_start(args, fmt);

    char *msgbuf = kzalloc(sizeof(char) * 1024, GFP_KERNEL);
    if (msgbuf == NULL) {
        pr_err("librefw: could not allocate log message buffer\n");
        return -ENOMEM;
    }

    int len = vsnprintf(msgbuf, 1024, fmt, args);
    if (len > 1024) {
        pr_warn("librefw: truncating log message\n");
    }

    int ret = lfw_nl_fn_log(level, ts.tv_sec, msgbuf);

    va_end(args);
    kfree(msgbuf);

    return ret;
}

int lfw_nl_fn_log(u8 level, u64 timestamp, char *msg)
{
    struct sk_buff *skb = genlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
    if (!skb) {
        pr_err("librefw: failed to allocate memory for genl message\n");
        return -ENOMEM;
    }

    void *hdr = genlmsg_put(skb, 0, 0, &lfw_nl_family, 0, LFW_NLX_LOG);
    if (!hdr) {
        pr_err("librefw: failed to allocate memory for genl header\n");
        nlmsg_free(skb);
        return -ENOMEM;
    }

    int ret = nla_put_u8(skb, LFW_NLA_LOG_LVL, level);
    if (ret) {
        pr_err("librefw: could not put loglevel into log message\n");
        goto err;
    }

    ret = nla_put_u64_64bit(skb, LFW_NLA_LOG_TS, timestamp, LFW_NLA_UNSPEC);
    if (ret) {
        pr_err("librefw: could not log put timestamp into log message\n");
        goto err;
    }

    ret = nla_put_string(skb, LFW_NLA_LOG_MSG, msg);
    if (ret) {
        pr_err("librefw: could not put message into log message\n");
        goto err;
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

err:
    genlmsg_cancel(skb, hdr);
    nlmsg_free(skb);
    return ret;
}
