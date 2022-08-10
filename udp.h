#ifndef UDP_H
#define UDP_H

#include <ctype.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <array>
#include <list>

namespace UDP
{
    class Udp
    {
    private:
        int udp_socket;
        sockaddr_in address;

    public:
        Udp(const std::string &_ip, uint16_t _port);
        ~Udp();

        template<typename T>
        void send_data(T *_data, size_t _size_array)
        {
            ssize_t ret = sendto(udp_socket, _data, _size_array  * sizeof(T), 0, (sockaddr*)&address, sizeof(sockaddr));
            if(ret < 0)
                printf("Error send UDP\n");
        }
    };
}
#endif // UDP_H
