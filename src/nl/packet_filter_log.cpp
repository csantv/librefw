#include "nl/packet_filter_log.hpp"
#include "nl_ops.h"

#include <netlink/genl/genl.h>

#include <iostream>

namespace lfw
{

PacketFilterListener::PacketFilterListener()
    : NetlinkMulticastBase(LFW_NL_FAMILY_NAME, "pkt_filter_log")
{
}

auto PacketFilterListener::on_message_received(nlattr_vec &tb) -> int
{
    std::cout << tb.size() << std::endl;
    return NL_OK;
}

} // namespace lfw
