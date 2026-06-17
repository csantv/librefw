#include "nl/packet_filter_log.hpp"
#include "nl_ops.h"

#include <netlink/genl/genl.h>
#include <netlink/netlink.h>

#include <ctime>
#include <iostream>

namespace lfw
{

PacketFilterListener::PacketFilterListener()
    : NetlinkMulticastBase(LFW_NL_FAMILY_NAME, "pkt_filter_log")
{
    struct timespec real_tp, boot_tp;
    clock_gettime(CLOCK_REALTIME, &real_tp);
    clock_gettime(CLOCK_MONOTONIC, &boot_tp);
    boot_ns =
        (uint64_t)(real_tp.tv_sec - boot_tp.tv_sec) * 1000000000ULL + (uint64_t)(real_tp.tv_nsec - boot_tp.tv_nsec);
}

auto PacketFilterListener::wait_for_messages_callback(struct nl_msg *msg, void *arg) -> int
{
    auto *ptr = static_cast<PacketFilterListener *>(arg);
    auto *hdr = nlmsg_hdr(msg);
    struct nlattr *pos, *nested_pos;
    int rem, nested_rem;
    nla_for_each_attr(pos, static_cast<struct nlattr *>(nlmsg_data(hdr)), nlmsg_datalen(hdr), rem)
    {
        nla_for_each_nested(nested_pos, pos, nested_rem)
        {
            switch (nla_type(nested_pos)) {
                case LFW_NLA_PKT_FILTER_LOG_TS: {
                    uint64_t ts = nla_get_u64(nested_pos) + ptr->boot_ns;
                    std::cout << ts << std::endl;
                    break;
                }
                case LFW_NLA_PKT_FILTER_LOG_SRC_IP:
                    break;
                case LFW_NLA_PKT_FILTER_LOG_DEST_IP:
                    break;
                case LFW_NLA_PKT_FILTER_LOG_SRC_PORT:
                    break;
                case LFW_NLA_PKT_FILTER_LOG_DEST_PORT:
                    break;
                case LFW_NLA_PKT_FILTER_LOG_PROTO:
                    break;
                case LFW_NLA_PKT_FILTER_LOG_TTL:
                    break;
                default:
                    std::cerr << "got unknown attribute type\n" << std::endl;
            }
        }
    }
    return NL_OK;
}

void PacketFilterListener::wait_for_messages()
{
    int ret = nl_socket_modify_cb(sock.get(), NL_CB_VALID, NL_CB_CUSTOM, wait_for_messages_callback, this);

    if (ret < 0) {
        throw std::system_error(std::error_code(-ret, std::generic_category()), "could not modify callback");
    }

    while (1) {
        nl_recvmsgs_default(sock.get());
    }
}

auto PacketFilterListener::on_message_received([[maybe_unused]] nlattr_vec &tb) -> int
{
    return 0;
}

} // namespace lfw
