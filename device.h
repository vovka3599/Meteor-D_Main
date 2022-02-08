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

#include "l_complex.h"
#include "dma_proxy.h"
#include "config.h"
#include "udp.h"
#include "register.h"

struct Queue_data
{
    std::vector<l_complex<int16_t>> rec_data;
};

static pthread_mutex_t lock;

class Device
{
private:
    int dma_rx_fd;
    struct dma_proxy_channel_interface *dma_rx_interface;

    int dma_tx_fd;
    struct dma_proxy_channel_interface *dma_tx_interface;
    Registers *control_struct;

    std::thread *dma_rx;
    int run_dma_rx;

    std::thread *dma_tx;
    int run_dma_tx;

    uint32_t tail_buf;

    std::queue<Queue_data> queue_tx;

    int udp_socket;

    uint64_t t_start, t_end;
    std::time_t t_result;
private:
    void Device_reset();
    void Init_DMA_RX(std::string dma_path);
    void Init_DMA_TX(std::string dma_path);
    void Init_control_struct();

    void Time_start();
    void Time_end();

    void DMA_Transmit();
    void DMA_Receive();

public:
    Device();
    void Init(std::string _dma_rx_path, std::string _dma_tx_path, int _udp_socket);
    void Start_DMA_RX();
    void Stop_DMA_RX();
    void Start_DMA_TX();
    void Stop_DMA_TX();
    void Set_control_struct();
};

#endif // DEVICE_H
