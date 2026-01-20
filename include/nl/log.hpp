#include "nl/base.hpp"

namespace lfw
{

class LogListener : private NetlinkBase
{
  public:
    LogListener();
    void wait_for_messages();

  private:
    int group_id = -1;

    static void callback(struct nl_msg *msg, void *arg);
};

} // namespace lfw
