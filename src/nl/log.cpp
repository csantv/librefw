#include "nl_ops.h"

#include "nl/log.hpp"

#include <netlink/genl/genl.h>

#include <iostream>

namespace lfw
{

LogListener::LogListener()
    : NetlinkMulticastBase(LFW_NL_FAMILY_NAME, "log")
{
}

auto LogListener::on_message_received(nlattr_vec& tb) -> int
{
    if (!tb.at(LFW_NLA_LOG_MSG)) {
        std::cerr << "msg attribute missing from message\n";
        return NL_SKIP;
    }

    std::string message {nla_get_string(tb[LFW_NLA_LOG_MSG])};
    uint64_t timestamp = nla_get_u64(tb[LFW_NLA_LOG_TS]);
    uint8_t level = nla_get_u8(tb[LFW_NLA_LOG_LVL]);

    std::cout << level << " " << timestamp << " " << message;

    return NL_OK;
}

} // namespace lfw
