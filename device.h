#ifndef DEVICE_H
#define DEVICE_H
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string>
#include <unistd.h>
#include <thread>
#include <vector>
#include <signal.h>
#include <complex>

#include "dma_proxy.h"
#include "config.h"
#include "udp.h"
#include "udp_receiver.h"
#include "register.h"

namespace METEOR {
    static const int dma_rx_len = 2048;
    static const int dma_tx_len = 2048;

    typedef struct Dma_rx
    {
        std::complex<int16_t> cn;
        float cor;
    }Dma_rx_t;

    typedef struct Dma_tx
    {
        std::complex<int16_t> cn;
    }Dma_tx_t;

    class Error : public std::runtime_error
    {
    public:
        Error(const std::string &er);
    };

    class Device
    {
    public:
        Device();
        void Init();
        void Set_Param(uint16_t _samp_freq,
                       bool _counter_test_en,
                       bool _cor_mode_file_test,
                       bool _adc_debug_en,
                       double _adc_freq,
                       double _adc_debug_freq,
                       double _adc_sys_clk,
                       bool _dac_debug_en,
                       double _dac_freq,
                       double _dac_sys_clk);
        void Init_DMA_RX(std::string dma_path);
        void Init_DMA_TX(std::string dma_path);
        void Init_DMA_BS(std::string dma_path);
        void Start_DMA_RX();
        void Stop_DMA_RX();
        void Stop_DMA_TX();
        void DMA_Transmit(Dma_tx_t *_data);
        void DMA_Transmit_BS(Dma_tx_t *_data);

        void* Get_DMA_Data();

        uint32_t Get_lost_sample();
        void Time_start();
        int Time_end();

        int InitBS(const int8_t *_preamble);

    private:
        void Device_reset();
        static void callback(int n, siginfo_t *info, void *unused);
        int Check_valid_buf(uint16_t _ptr_read, uint16_t _prt_dma);
        void *read();

    private:
        Registers *control_struct;

        int dma_rx_fd;
        struct dma_proxy_channel_interface *dma_rx_interface;

        int dma_tx_fd;
        struct dma_proxy_channel_interface *dma_tx_interface;

        int dma_bs_fd;
        struct dma_proxy_channel_interface *dma_bs_interface;

        uint64_t t_start, t_end;
        std::time_t t_result;

        static uint16_t ptr_read;
        static uint16_t ptr_dma;
    };
}

#endif // DEVICE_H
