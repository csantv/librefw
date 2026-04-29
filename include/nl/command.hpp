#pragma once

#include "nl/base.hpp"

#include <string>

namespace lfw
{

class CommandDispatcher : public NetlinkBase
{
  public:
    CommandDispatcher();

    void send_bogon_list(std::string filename);
    void set_under_attack(bool under_attack);
    void set_hcf_history();
};

} // namespace lfw
