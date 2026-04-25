#pragma once

#include "util/ptr.hpp"

#include <netlink/msg.h>
#include <netlink/socket.h>

#include <string>
#include <vector>

namespace lfw
{

using nl_msg_ptr = c_unique_ptr<struct nl_msg, nlmsg_free>;
using nlattr_vec = std::vector<struct nlattr *>;

class NetlinkBase
{
  public:
    NetlinkBase(const char *family_name, int bufsize = 4 * 1024 * 1024);
    virtual ~NetlinkBase() = default;

  protected:
    std::string family_name;
    c_unique_ptr<struct nl_sock, nl_socket_free> sock;
    int family_id = -1;

    auto make_message(int netlink_function) -> nl_msg_ptr;
};

class NetlinkMulticastBase : public NetlinkBase
{
  public:
    NetlinkMulticastBase(const char *family_name, const char *group_name, int bufsize = 4 * 1024 * 1024);
    void wait_for_messages();

    static auto parse_args(struct nl_msg *msg) -> nlattr_vec;

  protected:
    std::string group_name;
    int group_id;

    virtual auto on_message_received(nlattr_vec &tb) -> int = 0;
};

} // namespace lfw
