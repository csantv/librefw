#pragma once

#include "nl/base.hpp"

namespace lfw
{

class LogListener final : public NetlinkMulticastBase
{
  public:
    LogListener();

  private:
    auto on_message_received(nlattr_vec& tb) -> int override;
};

} // namespace lfw
