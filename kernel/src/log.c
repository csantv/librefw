#include "log.h"
#include "nl.h"
#include "nl_ops.h"

#include <linux/ktime.h>
#include <linux/slab.h>
#include <net/genetlink.h>

struct log_ctx {
    u8 level;
    u64 timestamp;
    char *msg;
};

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

    struct log_ctx ctx = {.level = level, .timestamp = ts.tv_sec, .msg = msgbuf};
    int ret = lfw_make_multicast_msg(LFW_NL_GROUP_LOG, LFW_NL_CMD_LOG, &ctx, lfw_build_log_msg);

    va_end(args);
    kfree(msgbuf);

    return ret;
}

int lfw_build_log_msg(struct sk_buff *skb, void *data)
{
    struct log_ctx *ctx = data;

    if (nla_put_u8(skb, LFW_NLA_LOG_LVL, ctx->level) ||
        nla_put_u64_64bit(skb, LFW_NLA_LOG_TS, ctx->timestamp, LFW_NLA_UNSPEC) ||
        nla_put_string(skb, LFW_NLA_LOG_MSG, ctx->msg)) {
        pr_err("librefw: could not build log message\n");
        return -EMSGSIZE;
    }

    return 0;
}
