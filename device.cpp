#include "device.h"

namespace METEOR {
    uint16_t Device::ptr_dma = 0;
    uint16_t Device::ptr_read = 0;

    Device::Device()
    {
    }

    void Device::Init()
    {
        uint32_t* status_reg;
        int fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
        status_reg = (uint32_t*)mmap(NULL, sizeof(Registers), PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, ADDR_REGISTERS);
        control_struct = (struct Registers*)status_reg;
        close(fd_mem);
    }

    void Device::Set_Param(uint16_t _samp_freq,
                           bool _counter_test_en,
                           bool _adc_debug_en,
                           double _adc_freq,
                           double _adc_debug_freq,
                           double _adc_sys_clk,
                           bool _dac_debug_en,
                           double _dac_freq,
                           double _dac_sys_clk)
    {
        control_struct->control.reset = 0;
        control_struct->control.adc_dith = 0;
        control_struct->control.adc_pga = 0;
        control_struct->control.adc_rand = 0;
        control_struct->control.adc_shdn = 0;

        control_struct->control.adc_debug = _adc_debug_en;

        control_struct->control.samp_rate = _samp_freq;

        control_struct->control.dac_real_data = !_dac_debug_en;

        control_struct->dac_dds_freq = (_dac_freq / _dac_sys_clk)*(1<<29);

        control_struct->adc_dds_freq = ((_adc_sys_clk - _adc_freq)/_adc_sys_clk)*(1<<27);
        control_struct->adc_dds_freq_debug = (_adc_debug_freq/_adc_sys_clk)*(1<<27);

        control_struct->control.counter_test = _counter_test_en;
    }

    void Device::Init_DMA_RX(std::string dma_path)
    {
        dma_rx_fd = open(dma_path.c_str(), O_RDWR);
        if (dma_rx_fd < 1)
        {
           printf("can't open receive loop device\n");
           exit(EXIT_FAILURE);
        }

        dma_rx_interface = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
                                        PROT_READ | PROT_WRITE, MAP_SHARED, dma_rx_fd, 0);
        if (dma_rx_interface == MAP_FAILED)
        {
            printf("Failed to mmap\n");
            exit(EXIT_FAILURE);
        }
        dma_rx_interface->length = BUFFER_SIZE*sizeof(d_buffer_t);

        struct sigaction sig;
        sig.sa_sigaction = &Device::callback;
        sig.sa_flags = SA_SIGINFO;
        int sigNum = SIGRTMIN;

        if( sigaction(sigNum, &sig, NULL) < 0 ){
            perror("sigaction");
            exit(1);
        }

        signal_parameters sig_params;
        sig_params.on = sigNum;
        sig_params.data = 0;
        sig_params.period = 1;
        ioctl(dma_rx_fd, PROXY_DMA_SET_SIGNAL, &sig_params);
    }

    void Device::Init_DMA_TX(std::string dma_path)
    {
        dma_tx_fd = open(dma_path.c_str(), O_RDWR);
        if (dma_tx_fd < 1)
        {
           printf("can't open receive loop device\n");
           exit(EXIT_FAILURE);
        }

        dma_tx_interface = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
                                        PROT_READ | PROT_WRITE, MAP_SHARED, dma_tx_fd, 0);
        if (dma_tx_interface == MAP_FAILED)
        {
            printf("Failed to mmap\n");
            exit(EXIT_FAILURE);
        }
        dma_tx_interface->length = BUFFER_SIZE*sizeof(d_buffer_t);
    }

    void Device::Start_DMA_RX()
    {
        ptr_dma = 0;
        ptr_read = 0;
        Device_reset();
        ioctl(dma_rx_fd, START_CYCLIC);
    }

    void Device::Stop_DMA_RX()
    {
        ioctl(dma_rx_fd, STOP_CYCLIC);
        munmap(dma_rx_interface, sizeof(struct dma_proxy_channel_interface));
        close(dma_rx_fd);
        control_struct->control.reset = 0;
    }

    void Device::Stop_DMA_TX()
    {
        munmap(dma_tx_interface, sizeof(struct dma_proxy_channel_interface));
        close(dma_tx_fd);
    }

    void Device::DMA_Transmit(d_buffer_t *_data)
    {
        memcpy(dma_tx_interface->buffer, _data, BUFFER_SIZE*sizeof(d_buffer));
        ioctl(dma_tx_fd, XFER);
    }

    d_buffer_t* Device::Get_DMA_Data()
    {
        return read();
    }

    uint32_t Device::Get_lost_sample()
    {
        return control_struct->lost_sample;
    }

    void Device::Time_start()
    {
        t_start = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    int Device::Time_end()
    {
        t_end = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        t_result = t_end - t_start;
        printf("time = %d\n", (int)t_result);
        return 0;
    }

    void Device::Device_reset()
    {
        control_struct->control.reset = 0;
        usleep(100);
        printf("State Reset = 0. Press any key...");
        getchar();
        control_struct->control.reset = 1;
        usleep(100);
        printf("State Reset = 1\n");
    }

    void Device::callback(int n, siginfo_t *info, void *unused)
    {
        ptr_dma = (ptr_dma + 1) % RX_BUFFER_COUNT;
    }

    int Device::Check_valid_buf(uint16_t _ptr_read, uint16_t _prt_dma)
    {
        int ret = _prt_dma - _ptr_read;
        return (ret) < 0 ? ret + RX_BUFFER_COUNT : ret;
    }

    d_buffer_t *Device::read()
    {
        if (Check_valid_buf(ptr_read, ptr_dma) < 1)
            return nullptr;
        d_buffer_t *ret = &dma_rx_interface->buffer[BUFFER_SIZE*ptr_read];
        ptr_read = (ptr_read + 1) % RX_BUFFER_COUNT;
        return ret;
    }
}
