#pragma once

#include "nl/base.hpp"

namespace lfw
{

class HcfListener final : public NetlinkMulticastBase
{
  public:
    HcfListener();

  private:
    auto on_message_received(nlattr_vec &tb) -> int override;
};

}; // namespace lfw
