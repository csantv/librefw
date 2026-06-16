#pragma once

#include "nl/base.hpp"

namespace lfw
{

class PacketFilterListener final : public NetlinkMulticastBase
{
  public:
    PacketFilterListener();

    void wait_for_messages() override;
    static auto wait_for_messages_callback(struct nl_msg *msg, void *arg) -> int;

  private:
    auto on_message_received(nlattr_vec &tb) -> int override;
};

}; // namespace lfw
