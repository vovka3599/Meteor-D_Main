#include "udp.h"

int create_udp_socket(int *s)
{
    char saddr[128];
    strcpy(saddr, ADDRESS_DEFAULT);

    struct addr_params arg;
    arg.saddr = &saddr[0];
    arg.port = PORT_DEFAULT;
    arg.fft_size = FFT_SIZE_DEFAULT;

    struct sockaddr_in sin;
    struct protoent* ppe;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;

    if((sin.sin_addr.s_addr = inet_addr(arg.saddr)) == INADDR_NONE)
    {
        printf("Can't get ip address %s  \n", arg.saddr);
        return -1;
    }

    if((sin.sin_port = htons(arg.port)) == 0)
    {
        printf("Can't get ip port %d \n", arg.port);
        return -1;
    }

    if((ppe = getprotobyname("udp")) == 0)
    {
        printf("Can't get protocol entry for udp \n");
        return -1;
    }

    *s = socket(PF_INET, SOCK_DGRAM, ppe->p_proto);
    if(*s < 0)
    {
        printf("Can't create socket: %s \n", strerror(errno));
        return -1;
    }
    printf("created UDP socket...\n");

    if(connect(*s, (struct sockaddr*)&sin, sizeof(sin)) < 0)
    {
        printf("Can't connect socket to %s, port %d: %s \n", arg.saddr, arg.port, strerror(errno));
        close(*s);
        return -1;
    }

    printf("Connected to UDP socket. address: %s, port %d \n", arg.saddr, arg.port);
    return 0;
}
