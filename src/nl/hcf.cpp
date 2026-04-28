#include "nl/hcf.hpp"
#include "nl_ops.h"
#include "sql/hcf_ipv4_history.h"

#include <arpa/inet.h>

#include <netlink/genl/genl.h>
#include <sqlpp23/sqlite3/sqlite3.h>

#include <array>
#include <cstring>
#include <format>
#include <iostream>
#include <vector>

namespace lfw
{

using sqlpp::sqlite3::insert_or_ignore;

HcfListener::HcfListener()
    : NetlinkMulticastBase(LFW_NL_FAMILY_NAME, "hcf")
{
}

auto HcfListener::on_message_received(nlattr_vec &tb) -> int
{
    uint32_t source_ip = nla_get_u32(tb[LFW_NLA_HCF_IP]);
    uint8_t ttl = nla_get_u8(tb[LFW_NLA_HCF_TTL]);
    uint8_t hc = nla_get_u8(tb[LFW_NLA_HCF_HC]);
    std::vector<unsigned char> ip_addr(4, 0);
    std::memcpy(ip_addr.data(), &source_ip, 4);

    std::array<char, INET_ADDRSTRLEN> buf{};
    struct in_addr res{};
    res.s_addr = htonl(source_ip);

    if (inet_ntop(AF_INET, &res, buf.data(), INET_ADDRSTRLEN) == nullptr) {
        std::cerr << "Invalid address\n";
        return NL_SKIP;
    }

    std::string_view ip_addr_str{buf.data()};

    try {
        manager.execute([&ip_addr, &ttl, &hc, ip_addr_str](db::connection &db) {
            sql::hcf_ipv4_history tab{};
            db(insert_or_ignore().into(tab).set(tab.ttl = ttl, tab.hc = hc, tab.ip_addr = ip_addr,
                                                tab.ip_addr_str = ip_addr_str));
        });
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
    }

    auto str = std::format("[hcf] inserted new ip {} tll={} hc={}", ip_addr_str, ttl, hc);
    std::cout << str << std::endl;

    return NL_OK;
}

}; // namespace lfw
