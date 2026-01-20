#include "nl_ops.h"

#include "nl/log.hpp"

#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>

#include <array>
#include <iostream>

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
    int ret = nl_socket_modify_cb(sock.get(), NL_CB_VALID, NL_CB_CUSTOM, callback, this);
    if (ret < 0) {
        throw std::system_error(std::error_code(-ret, std::generic_category()), "could not modify callback");
    }

    while (1) {
        nl_recvmsgs_default(sock.get());
    }
}

auto LogListener::callback(struct nl_msg *msg, void *arg) -> int
{
    auto *hdr = static_cast<struct genlmsghdr *>(nlmsg_data(nlmsg_hdr(msg)));
    std::array<struct nlattr *, LFW_NLA_MAX + 1> tb{};

    int ret = nla_parse(tb.data(), LFW_NLA_MAX, genlmsg_attrdata(hdr, 0), genlmsg_attrlen(hdr, 0), nullptr);
    if (ret) {
        std::cerr << "unable to parse message: " << std::error_code(-ret, std::generic_category()).message()
                  << std::endl;
        return NL_SKIP;
    }

    if (!tb.at(LFW_NLA_LOG_MSG)) {
        std::cerr << "msg attribute missing from message" << std::endl;
        return NL_SKIP;
    }

    std::cout << nla_get_string(tb[LFW_NLA_LOG_MSG]) << std::endl;

    return NL_OK;
}

} // namespace lfw
