#include "util/util.hpp"

#include <array>

#include <arpa/inet.h>

namespace lfw
{

auto be32_ip_to_string(uint32_t addr) -> std::string
{
    std::array<char, INET_ADDRSTRLEN> buf{};
    struct in_addr res{};
    res.s_addr = addr;

    if (inet_ntop(AF_INET, &res, buf.data(), INET_ADDRSTRLEN) == nullptr) {
        return {};
    }

    return buf.data();
}

} // namespace lfw
