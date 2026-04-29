#pragma once

#include "db/manager.hpp"
#include "nl/base.hpp"

namespace lfw
{

class HcfListener final : public NetlinkMulticastBase
{
  public:
    HcfListener();

    void set_ip_history();

  protected:
    auto on_message_received(nlattr_vec &tb) -> int override;

  private:
    db::DbManager manager;
};

}; // namespace lfw
