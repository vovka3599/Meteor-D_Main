#include "udp_receiver.h"


udp_receiver::udp_receiver(std::string addr, uint16_t port, int _size_data)
{
    size_data = _size_data;
    m_socket = socket(AF_INET, SOCK_DGRAM, 0);
    myAddress.sin_family = AF_INET;
    myAddress.sin_port = htons(port);
    inet_aton(addr.c_str(), (in_addr*)&myAddress.sin_addr.s_addr);

    if(bind(m_socket, (struct sockaddr*) &myAddress, sizeof(myAddress)) < 0){
        std::runtime_error("error bind udp\n");
    }

    m_buffer = new char[size_data];
}

udp_receiver::~udp_receiver()
{
    delete [] m_buffer;
    close(m_socket);
}

char* udp_receiver::get_data()
{
    int bytesReceived = recv(m_socket, (void*)m_buffer, size_data, MSG_DONTWAIT);
    if(bytesReceived != size_data){
        if(bytesReceived > 0)
            printf("got strange %d bytes %d\n", bytesReceived, size_data);
    }
    return  m_buffer;
}
