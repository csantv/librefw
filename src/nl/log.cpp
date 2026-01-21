#include "nl_ops.h"

#include "nl/log.hpp"

#include <netlink/genl/genl.h>

#include <array>
#include <iostream>

namespace lfw
{

LogListener::LogListener()
    : NetlinkMulticastBase(LFW_NL_FAMILY_NAME, LFW_NL_MC_GRP_NAME)
{
}

auto LogListener::on_message_received(struct nl_msg *msg) -> int
{
    auto *hdr = static_cast<struct genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
    std::array<struct nlattr *, LFW_NLA_MAX + 1> tb{};

    int ret = nla_parse(tb.data(), LFW_NLA_MAX, genlmsg_attrdata(hdr, 0), genlmsg_attrlen(hdr, 0), nullptr);
    if (ret) {
        std::cerr << "unable to parse message: " << std::error_code(-ret, std::generic_category()).message()
                  << std::endl;
        return NL_SKIP;
    }

    if (!tb.at(LFW_NLA_LOG_MSG)) {
        std::cerr << "msg attribute missing from message" << std::endl;
        return NL_SKIP;
    }

    std::cout << nla_get_string(tb[LFW_NLA_LOG_MSG]) << std::endl;

    return NL_OK;
}

} // namespace lfw
