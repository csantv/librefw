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

    std::cout << nla_get_string(tb[LFW_NLA_LOG_MSG]) << std::endl;

    return NL_OK;
}

} // namespace lfw
