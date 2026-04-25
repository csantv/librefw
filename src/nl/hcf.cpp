#include "nl/hcf.hpp"
#include "nl_ops.h"

#include <netlink/genl/genl.h>

#include <iostream>

namespace lfw
{

auto HcfListener::on_message_received(struct nl_msg *msg) -> int
{
    nlattr_vec tb;
    try {
        tb = parse_args(msg);
    } catch (std::exception &ex) {
        std::cerr << ex.what() << '\n';
        return NL_SKIP;
    }

    return NL_OK;
}

}; // namespace lfw
