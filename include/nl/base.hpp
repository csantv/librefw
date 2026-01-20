#pragma once

#include "util/ptr.hpp"

#include <netlink/socket.h>

#include <string>

struct nl_sock;

namespace lfw
{

class NetlinkBase
{
  public:
    NetlinkBase(const char *family_name, int bufsize = 4 * 1024 * 1024);

  protected:
    std::string family_name;
    c_unique_ptr<struct nl_sock, nl_socket_free> sock;
    int family_id = -1;
};

} // namespace lfw
