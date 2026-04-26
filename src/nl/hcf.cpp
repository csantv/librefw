#include "nl/hcf.hpp"
#include "nl_ops.h"

#include <arpa/inet.h>
#include <netlink/genl/genl.h>

#include <array>
#include <format>
#include <iostream>

namespace lfw
{

HcfListener::HcfListener()
    : NetlinkMulticastBase(LFW_NL_FAMILY_NAME, "hcf")
{
}

auto HcfListener::on_message_received([[maybe_unused]] nlattr_vec &tb) -> int
{
    uint32_t source_ip = nla_get_u32(tb[LFW_NLA_HCF_IP]);
    uint8_t ttl = nla_get_u8(tb[LFW_NLA_HCF_TTL]);
    uint8_t hc = nla_get_u8(tb[LFW_NLA_HCF_HC]);

    std::array<char, INET_ADDRSTRLEN> buf {};

    struct in_addr res;
    res.s_addr = htonl(source_ip);

    if (inet_ntop(AF_INET, &res, buf.data(), INET_ADDRSTRLEN) == nullptr) {
        std::cerr << "Invalid address\n";
        return NL_SKIP;
    }

    std::string_view ip_str{buf.data()};

    auto str = std::format("[hcf] new ip {} tll={} hc={}", ip_str, ttl, hc);
    std::cout << str << std::endl;

    return NL_OK;
}

}; // namespace lfw
