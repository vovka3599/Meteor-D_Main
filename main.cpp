#include <stdio.h>
#include <csignal>
#include "device.h"
#include "lmk.h"

static volatile std::sig_atomic_t exit_thread;
void sigintHandler(int sig)
{
    exit_thread = 1;
}

int main(int argc, char* argv[])
{
     std::signal(SIGINT, sigintHandler);
#if UDP_SEND
    int rc;
    int udp_socket;
    rc = create_udp_socket(&udp_socket);
    if(rc < 0)
    {
        printf("Create udp socket fail\n");
        exit(0);
    }
#endif

    Device *dev = new Device();
#if UDP_SEND
    dev->Init(DEVICE_RX, DEVICE_TX, udp_socket);
#else
    dev->Init(DEVICE_RX, DEVICE_TX, 0);
#endif
    dev->Set_control_struct();

    LMK *lmk = new LMK();
    lmk->lmk_init();

    dev->Start_DMA_RX();

#if DAC_SEND
    dev->Start_DMA_TX();
#endif

    while(exit_thread == 0)
    {
        usleep(100);
    }

    dev->Stop_DMA_RX();

#if DAC_SEND
    dev->Stop_DMA_TX();
#endif

#if UDP_SEND
    close(rc);
#endif

    delete lmk;
    delete dev;

    return 0;
}
