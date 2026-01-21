#pragma once

#include "util/ptr.hpp"

#include <netlink/socket.h>

#include <string>

namespace lfw
{

class NetlinkBase
{
  public:
    NetlinkBase(const char *family_name, int bufsize = 4 * 1024 * 1024);
    virtual ~NetlinkBase() = default;

  protected:
    std::string family_name;
    c_unique_ptr<struct nl_sock, nl_socket_free> sock;
    int family_id = -1;
};

class NetlinkMulticastBase : public NetlinkBase
{
  public:
    NetlinkMulticastBase(const char *family_name, const char *group_name, int bufsize = 4 * 1024 * 1024);
    void wait_for_messages();

  protected:
    std::string group_name;
    int group_id;
    virtual auto on_message_received(struct nl_msg *msg) -> int = 0;
};

} // namespace lfw
