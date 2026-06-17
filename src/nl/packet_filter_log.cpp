#include "nl/packet_filter_log.hpp"
#include "nl_ops.h"
#include "util/util.hpp"

#include <netlink/genl/genl.h>
#include <netlink/netlink.h>

#include <ctime>
#include <iostream>

namespace lfw
{

struct PacketFilterEvent {
    uint64_t ts;
    uint32_t source_ip;
    uint32_t dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    uint8_t proto;
    uint8_t ttl;

    void print()
    {
        std::cout << "[" << ts << "]" << be32_ip_to_string(source_ip) << ":" << ntohs(source_port) << "=>"
                  << be32_ip_to_string(dest_ip) << ":" << ntohs(dest_port) << " proto:" << +proto  << " ttl:" << +ttl
                  << std::endl;
    }
};

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
        if (nla_type(pos) != LFW_NLA_PKT_FILTER_LOG) {
            continue;
        }
        PacketFilterEvent event{};
        nla_for_each_nested(nested_pos, pos, nested_rem)
        {
            switch (nla_type(nested_pos)) {
                case LFW_NLA_PKT_FILTER_LOG_TS:
                    event.ts = nla_get_u64(nested_pos) + ptr->boot_ns;
                    break;
                case LFW_NLA_PKT_FILTER_LOG_SRC_IP:
                    event.source_ip = nla_get_u32(nested_pos);
                    break;
                case LFW_NLA_PKT_FILTER_LOG_DEST_IP:
                    event.dest_ip = nla_get_u32(nested_pos);
                    break;
                case LFW_NLA_PKT_FILTER_LOG_SRC_PORT:
                    event.source_port = nla_get_u16(nested_pos);
                    break;
                case LFW_NLA_PKT_FILTER_LOG_DEST_PORT:
                    event.dest_port = nla_get_u16(nested_pos);
                    break;
                case LFW_NLA_PKT_FILTER_LOG_PROTO:
                    event.proto = nla_get_u8(nested_pos);
                    break;
                case LFW_NLA_PKT_FILTER_LOG_TTL:
                    event.ttl = nla_get_u8(nested_pos);
                    break;
                default:
                    std::cerr << "got unknown attribute type"<< std::endl;
            }
        }
        event.print();
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
