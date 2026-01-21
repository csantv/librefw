#include "nl/command.hpp"
#include "nl_ops.h"

#include <arpa/inet.h>
#include <netlink/genl/genl.h>

#include <fstream>
#include <iostream>

namespace lfw
{

CommandDispatcher::CommandDispatcher()
    : NetlinkBase(LFW_NL_FAMILY_NAME)
{
}

void CommandDispatcher::send_bogon_list(std::string filename)
{
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Could not open " << filename << '\n';
        return;
    }

    c_unique_ptr<struct nl_msg, nlmsg_free> msg{nlmsg_alloc()};

    void *hdr =
        genlmsg_put(msg.get(), NL_AUTO_PORT, NL_AUTO_SEQ, family_id, 0, 0, LFW_NLX_BOGON_SET, LFW_NL_FAMILY_VER);
    if (!hdr) {
        std::cout << "genlmsg_put failed" << std::endl;
        return;
    }

    std::string line;
    unsigned int num_lines = 0;
    while (std::getline(file, line)) {
        auto slash_pos = line.find('/');
        std::string ip_prefix = line.substr(0, slash_pos);
        unsigned short ip_prefix_len = std::stoi(line.substr(slash_pos + 1));

        struct in_addr addr;
        inet_pton(AF_INET, ip_prefix.c_str(), &addr);

        struct nlattr *container = nla_nest_start(msg.get(), LFW_NLA_IP_PREFIX);
        if (nla_put_u32(msg.get(), LFW_NLA_N_IP_ADDR, addr.s_addr) < 0 ||
            nla_put_u8(msg.get(), LFW_NLA_N_IP_PREFIX_LEN, ip_prefix_len) < 0) {
            std::cerr << "failed to pack structures\n";
            nla_nest_cancel(msg.get(), container);
            break;
        }
        nla_nest_end(msg.get(), container);

        num_lines++;
    }

    nla_put_u32(msg.get(), LFW_NLA_NUM_IP_PREFIX, num_lines);

    int ret = nl_send_auto(sock.get(), msg.get());
    if (ret < 0) {
        std::cout << "nl_send_auto failed - " << ret << std::endl;
    } else {
        std::cout << "sent bogon list - " << ret << std::endl;
    }
}

} // namespace lfw
