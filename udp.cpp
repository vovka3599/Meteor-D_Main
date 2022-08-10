#include "udp.h"

namespace UDP
{
    Udp::Udp(const std::string &_ip, uint16_t _port)
    {
        udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if(udp_socket < 0)
            throw std::runtime_error("failed to create udp socket");
        address.sin_family = AF_INET;
        address.sin_port = htons(_port);
        address.sin_addr.s_addr = inet_addr(_ip.c_str());
    }

    Udp::~Udp()
    {
        close(udp_socket);
    }
}


