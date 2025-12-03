#pragma once

#include "ptr.hpp"

#include <string>

#include <netlink/netlink.h>

namespace lfw::nl {

auto echo_reply_handler(struct nl_msg *msg, void *arg) -> int;

class sock {
public:
    explicit sock(const char* family_name);
    void send_bogon_list(std::string filename);
    void wait_for_messages();

private:
    c_unique_ptr<struct nl_sock, nl_socket_free> sk;
    c_unique_ptr<struct nl_sock, nl_socket_free> msk;
    int m_family_id = -1;
    int m_mcast_grp = -1;
};

};
