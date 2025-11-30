#include "util/nl.hpp"
#include "util/ptr.hpp"
#include "nl_ops.h"

#include <arpa/inet.h>

#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>

#include <array>
#include <iostream>
#include <system_error>
#include <string>
#include <fstream>

namespace lfw::nl {

sock::sock(const char* family_name):
    sk(nl_socket_alloc())
{
    if (sk == nullptr) {
        throw std::system_error(std::make_error_code(std::errc::not_enough_memory), "nl_socket_alloc");
    }
    genl_connect(sk.get());
    m_family_id = genl_ctrl_resolve(sk.get(), family_name);
    if (m_family_id < 0) {
        throw std::system_error(std::make_error_code(std::errc::invalid_argument), "genl_ctrl_resolve");
    }

    int ret = nl_socket_modify_cb(sk.get(), NL_CB_VALID, NL_CB_CUSTOM, echo_reply_handler, nullptr);
    if (ret < 0) {
        throw std::system_error(std::error_code(-ret, std::generic_category()), "nl_socket_modify_cb");
    }

    const size_t bufsize = 4 * 1024 * 1024;
    ret = nl_socket_set_buffer_size(sk.get(), bufsize, bufsize);
    if (ret < 0) {
        throw std::system_error(std::error_code(-ret, std::generic_category()), "could not set buffer size");
    }
    nlmsg_set_default_size(bufsize);
}

void sock::send_echo_msg()
{
    c_unique_ptr<struct nl_msg, nlmsg_free> msg{nlmsg_alloc()};

    void *hdr = genlmsg_put(msg.get(), NL_AUTO_PORT, NL_AUTO_SEQ, m_family_id, 0, 0, LFW_NL_CMD_ECHO, LFW_NL_FAMILY_VER);
    if (!hdr) {
        std::cerr << "genlmsg_put failed" << std::endl;
        return;
    }

    int ret = nla_put_string(msg.get(), LFW_NL_A_MSG, "Hello from C++, Netlink!");
    if (ret < 0) {
        std::cerr << "nla_put_string failed" << std::endl;
        return;
    }

    nl_send_auto(sk.get(), msg.get());
    nl_recvmsgs_default(sk.get());
}

auto echo_reply_handler(struct nl_msg *msg, [[maybe_unused]] void *arg) -> int {
    auto *hdr = static_cast<struct genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
    std::array<struct nlattr*, LFW_NL_A_MAX + 1> tb{};

    int ret = nla_parse(tb.data(), LFW_NL_A_MAX, genlmsg_attrdata(hdr, 0), genlmsg_attrlen(hdr, 0), nullptr);
    if (ret) {
        std::cerr << "unable to parse message: " << std::error_code(-ret, std::generic_category()).message() << std::endl;
        return NL_SKIP;
    }
    if (!tb.at(LFW_NL_A_MSG)) {
        std::cerr << "msg attribute missing from message" << std::endl;
        return NL_SKIP;
    }

    std::cout << "message received: " << nla_get_string(tb[LFW_NL_A_MSG]) << std::endl;

    return NL_OK;
}

void sock::send_bogon_list(std::string filename)
{
    std::ifstream file(filename);

    if (!file) {
        std::cerr << "Could not open " << filename << '\n';
        return;
    }

    c_unique_ptr<struct nl_msg, nlmsg_free> msg{nlmsg_alloc()};

    void *hdr = genlmsg_put(msg.get(), NL_AUTO_PORT, NL_AUTO_SEQ, m_family_id, 0, 0, LFW_NL_CMD_BOGON_SET, LFW_NL_FAMILY_VER);
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

        struct nlattr *container = nla_nest_start(msg.get(), LFW_NL_A_IP_PREFIX);
        if (nla_put_u32(msg.get(), LFW_NL_A_N_IP_ADDR, addr.s_addr) < 0 ||
            nla_put_u8(msg.get(), LFW_NL_A_N_IP_PREFIX_LEN, ip_prefix_len) < 0) {
            std::cerr << "failed to pack structures\n";
            nla_nest_cancel(msg.get(), container);
            break;
        }
        nla_nest_end(msg.get(), container);

        num_lines++;
    }

    nla_put_u32(msg.get(), LFW_NL_A_NUM_IP_PREFIX, num_lines);


    int ret = nl_send_auto(sk.get(), msg.get());
    if (ret < 0) {
        std::cout << "nl_send_auto failed - " << ret << std::endl;
    } else {
        std::cout << "sent bogon list - " << ret << std::endl;
    }
    nl_recvmsgs_default(sk.get());

    /*


    nla_put_u32(msg.get(), LFW_NL_A_NUM_IP_PREFIX, 3);

    for (int i = 0; i < 3; i++) {
        if (container == nullptr) {
            std::cout << "nla_nest_start failed" << std::endl;
            return;
        }

        if (nla_put_u32(msg.get(), LFW_NL_A_N_IP_ADDR, 1 + i) < 0 ||
            nla_put_u8(msg.get(), LFW_NL_A_N_IP_PREFIX_LEN, 24 + i) < 0) {
            std::cout << "failed to pack structures" << std::endl;
        };

        nla_nest_end(msg.get(), container);
    }*/

}

}
