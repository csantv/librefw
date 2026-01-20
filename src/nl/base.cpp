#include "nl/base.hpp"

#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>

namespace lfw
{

NetlinkBase::NetlinkBase(const char *family_name, int bufsize)
    : family_name(family_name),
      sock(nl_socket_alloc())
{
    if (!sock) {
        throw std::system_error(std::make_error_code(std::errc::not_enough_memory), "nl_socket_alloc");
    }

    genl_connect(sock.get());

    family_id = genl_ctrl_resolve(sock.get(), family_name);
    if (family_id < 0) {
        throw std::system_error(std::error_code(-family_id, std::generic_category()),
                                "could not resolve netlink family");
    }

    int ret = nl_socket_set_buffer_size(sock.get(), bufsize, bufsize);
    if (ret < 0) {
        throw std::system_error(std::error_code(-ret, std::generic_category()), "could not set buffer size");
    }
}

} // namespace lfw
