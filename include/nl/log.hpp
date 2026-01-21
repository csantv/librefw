#pragma once

#include "nl/base.hpp"

namespace lfw
{

class LogListener final : public NetlinkMulticastBase
{
  public:
    LogListener();

  private:
    int group_id = -1;
    auto on_message_received(struct nl_msg *msg) -> int override;
};

} // namespace lfw
