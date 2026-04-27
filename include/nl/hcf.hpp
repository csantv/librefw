#pragma once

#include "nl/base.hpp"
#include "db/manager.hpp"

namespace lfw
{

class HcfListener final : public NetlinkMulticastBase
{
  public:
    HcfListener();

  protected:
    auto on_message_received(nlattr_vec &tb) -> int override;

private:
    db::DbManager manager;
};

}; // namespace lfw
