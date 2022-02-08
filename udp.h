#ifndef UDP_H
#define UDP_H

#include <ctype.h>
#include <iostream>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#include "config.h"

struct addr_params
{
    char* saddr;            // UDP Source IP address as a string
    uint16_t port;          // Port on which UDP Source is listening
    int dma_buffer_size;    // The size of one block of memory for a DMA transaction
    int dma_buffer_count;   // The number of memory blocks allocated for a DMA transaction
    int fft_size;           // The number of complex samples for the FFT that will be written to the UDP packet
};

typedef struct info_get_data
{
    int loop_count;
    int size_buf;
    struct file *filp;
    char *userbuf;
}INFO;

/**
 * @brief Create_udp_socket Create udp socket
 *
 * @param s                 udp socket
 * @return int              execution result
 */
int create_udp_socket(int *s);

#endif // UDP_H
