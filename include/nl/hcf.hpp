#pragma once

#include "nl/base.hpp"

namespace lfw
{

class HcfListener final : public NetlinkMulticastBase
{
  public:
    HcfListener();

  private:
    auto on_message_received(struct nl_msg *msg) -> int override;
};

}; // namespace lfw
