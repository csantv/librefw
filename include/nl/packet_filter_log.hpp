#pragma once

#include "nl/base.hpp"

namespace lfw
{

class PacketFilterListener final : public NetlinkMulticastBase
{
  public:
    PacketFilterListener();

  private:
    auto on_message_received(nlattr_vec &tb) -> int override;
};

}; // namespace lfw
