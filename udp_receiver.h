#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include <ctype.h>
#include <iostream>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "config.h"

class udp_receiver
{
private:
    int m_socket;
    struct sockaddr_in myAddress;
    char* m_buffer;
    int size_data;
public:
    udp_receiver(std::string addr, uint16_t port, int _size_data);
    ~udp_receiver();
    char* get_data();
};

#endif // UDP_RECEIVER_H
