#include "nl_ops.h"
#include "util/nl.hpp"

int main()
{
    lfw::nl::sock sk(LFW_NL_FAMILY_NAME);
    sk.send_echo_msg();
    return 0;
}
