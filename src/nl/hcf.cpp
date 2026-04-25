#include "nl/hcf.hpp"
#include "nl_ops.h"

#include <netlink/genl/genl.h>

namespace lfw
{

HcfListener::HcfListener()
    : NetlinkMulticastBase(LFW_NL_FAMILY_NAME, "hcf")
{
}

auto HcfListener::on_message_received([[maybe_unused]] nlattr_vec &tb) -> int
{

    return NL_OK;
}

}; // namespace lfw
