#include "udp_receiver.h"

udp_receiver::udp_receiver(std::string addr, uint16_t port, int _size_data)
{
    size_data = _size_data;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(udp_socket < 0)
        throw std::runtime_error("failed to create udp socket");
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(addr.c_str());
    inet_aton(addr.c_str(), (in_addr*)&address.sin_addr.s_addr);

    if(bind(udp_socket, (struct sockaddr*) &address, sizeof(address)) < 0){
        std::runtime_error("error bind udp\n");
    }

    m_buffer = new l_complex<short>[size_data];
}

udp_receiver::~udp_receiver()
{
    delete [] m_buffer;
    close(udp_socket);
}

int udp_receiver::available_data()
{
    socklen_t remoteAddressLength = 0;
    int bytesReceived = recvfrom(
        udp_socket,
        (void*)m_buffer,
        size_data*sizeof(l_complex<short>),
        MSG_DONTWAIT,
        (sockaddr*)&address,
        &remoteAddressLength
    );

    if(bytesReceived != size_data*sizeof(l_complex<short>)){
        if(bytesReceived > 0)
            printf("got strange %d bytes %d\n", bytesReceived, size_data*sizeof(l_complex<short>));
    }
    return bytesReceived;
}

l_complex<short>* udp_receiver::get_data()
{
    return m_buffer;
}
