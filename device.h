#ifndef DEVICE_H
#define DEVICE_H
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>
#include <unistd.h>
#include <thread>
#include <fstream>
#include <vector>
#include <queue>
#include <mutex>
#include <pthread.h>
#include <signal.h>
#include <functional>

#include "l_complex.h"
#include "dma_proxy.h"
#include "config.h"
#include "udp.h"
#include "udp_receiver.h"
#include "register.h"

namespace METEOR {
    class Device
    {
    public:
        Device();
        void Init_DMA_RX(std::string dma_path);
        void Init_DMA_TX(std::string dma_path);
        void Start_DMA_RX();
        void Stop_DMA_RX();
        void Stop_DMA_TX();
        void DMA_Transmit(d_buffer_t *_data);
        d_buffer_t* Get_DMA_Data();
        uint32_t Get_lost_sample();
        void Time_start();
        int Time_end();

    private:
        int dma_rx_fd;
        struct dma_proxy_channel_interface *dma_rx_interface;

        int dma_tx_fd;
        struct dma_proxy_channel_interface *dma_tx_interface;

        Registers *control_struct;

        uint64_t t_start, t_end;
        std::time_t t_result;

        static uint16_t ptr_read;
        static uint16_t ptr_dma;

    private:
        void Device_reset();
        void Init_control_struct();
        void Set_control_struct();

        static void callback(int n, siginfo_t *info, void *unused);
        int Check_valid_buf(uint16_t _ptr_read, uint16_t _prt_dma);
        d_buffer_t *read();
    };
}

#endif // DEVICE_H
