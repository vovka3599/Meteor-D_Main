#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include <ctype.h>
#include <iostream>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "l_complex.h"

#include "config.h"

class udp_receiver
{
private:
    int udp_socket;
    sockaddr_in address;
    l_complex<short>* m_buffer;
    int size_data;
public:
    udp_receiver(std::string addr, uint16_t port, int _size_data);
    ~udp_receiver();
    int available_data();
    l_complex<short>* get_data();
};

#endif // UDP_RECEIVER_H
