#pragma once

#include "ptr.hpp"

#include <netlink/netlink.h>

namespace lfw::nl {

auto echo_reply_handler(struct nl_msg *msg, void *arg) -> int;

class sock {
public:
    explicit sock(const char* family_name);
    void send_echo_msg();

    void send_bogon_list();

private:
    c_unique_ptr<struct nl_sock, nl_socket_free> sk;
    int m_family_id = -1;
};

};
