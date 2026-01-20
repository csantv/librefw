#include "nl_ops.h"

#include "nl/log.hpp"

#include <netlink/genl/ctrl.h>

namespace lfw
{

LogListener::LogListener()
    : NetlinkBase(LFW_NL_FAMILY_NAME)
{
    nl_socket_disable_seq_check(sock.get());
    group_id = genl_ctrl_resolve_grp(sock.get(), family_name.c_str(), LFW_NL_MC_GRP_NAME);
    if (group_id < 0) {
        throw std::system_error(std::error_code(-group_id, std::generic_category()), "could not resolve group name");
    }

    int ret = nl_socket_add_membership(sock.get(), group_id);
    if (ret < 0) {
        throw std::system_error(std::error_code(-ret, std::generic_category()), "could not subscribe to group");
    }
}

void LogListener::wait_for_messages()
{
    while (1) {
    }
    nl_recvmsgs_default(sock.get());
}

void LogListener::callback(struct nl_msg *msg, void *arg)
{
}

} // namespace lfw
