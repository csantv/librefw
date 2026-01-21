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

NetlinkMulticastBase::NetlinkMulticastBase(const char *family_name, const char *group_name, int bufsize)
    : NetlinkBase(family_name, bufsize),
      group_name(group_name)
{
    nl_socket_disable_seq_check(sock.get());
    group_id = genl_ctrl_resolve_grp(sock.get(), family_name, group_name);
    if (group_id < 0) {
        throw std::system_error(std::error_code(-group_id, std::generic_category()), "could not resolve group name");
    }

    int ret = nl_socket_add_membership(sock.get(), group_id);
    if (ret < 0) {
        throw std::system_error(std::error_code(-ret, std::generic_category()), "could not subscribe to group");
    }
}

void NetlinkMulticastBase::wait_for_messages()
{
    auto fn = [](struct nl_msg *msg, void *arg) -> int {
        auto *ptr = static_cast<NetlinkMulticastBase *>(arg);
        return ptr->on_message_received(msg);
    };
    int ret = nl_socket_modify_cb(sock.get(), NL_CB_VALID, NL_CB_CUSTOM, fn, this);

    if (ret < 0) {
        throw std::system_error(std::error_code(-ret, std::generic_category()), "could not modify callback");
    }

    while (1) {
        nl_recvmsgs_default(sock.get());
    }
}

} // namespace lfw
