#include "device.h"

namespace METEOR {
    uint16_t Device::ptr_dma = 0;
    uint16_t Device::ptr_read = 0;

    Device::Device(){}

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
                           bool _cor_mode_file_test,
                           bool _adc_debug_en,
                           double _adc_freq,
                           double _adc_debug_freq,
                           double _adc_sys_clk,
                           bool _dac_debug_en,
                           double _dac_freq,
                           double _dac_sys_clk)
    {
        control_struct->control.reset = 0;
        control_struct->control.samp_rate = _samp_freq;
        control_struct->control.adc_dith = 0;
        control_struct->control.adc_pga = 0;
        control_struct->control.adc_rand = 0;
        control_struct->control.adc_shdn = 0;
        control_struct->control.adc_debug = _adc_debug_en;
        control_struct->control.dac_real_data = !_dac_debug_en;
        control_struct->control.counter_test = _counter_test_en;
        control_struct->control.start_analize = 0;
        control_struct->control.bs_select_data = !_cor_mode_file_test;

        control_struct->dac_dds_freq = (_dac_freq / _dac_sys_clk)*(1<<29);

        control_struct->adc_dds_freq = ((_adc_sys_clk - _adc_freq)/_adc_sys_clk)*(1<<27);
        control_struct->adc_dds_freq_debug = (_adc_debug_freq/_adc_sys_clk)*(1<<27);

        control_struct->dma_ptk_len = dma_rx_len;

        control_struct->reference_energy = 20470;
        control_struct->data_compare = 0.1;
    }

    void Device::Init_DMA_RX(std::string dma_path)
    {
        dma_rx_fd = open(dma_path.c_str(), O_RDWR);
        if (dma_rx_fd < 1){
            throw Error("can't open receive loop device\n");
        }

        dma_rx_interface = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
                                        PROT_READ | PROT_WRITE, MAP_SHARED, dma_rx_fd, 0);
        if (dma_rx_interface == MAP_FAILED){
           throw Error("Failed to mmap\n");
        }
        dma_rx_interface->length = dma_rx_len*sizeof(Dma_rx_t);

        struct sigaction sig;
        sig.sa_sigaction = &Device::callback;
        sig.sa_flags = SA_SIGINFO;
        int sigNum = SIGRTMIN;

        if(sigaction(sigNum, &sig, NULL) < 0 ){
            throw Error("sigaction\n");
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
        if (dma_tx_fd < 1){
            throw Error("can't open TX loop device\n");
        }

        dma_tx_interface = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
                                        PROT_READ | PROT_WRITE, MAP_SHARED, dma_tx_fd, 0);
        if (dma_tx_interface == MAP_FAILED){
            throw Error("Failed to mmap\n");
        }
        dma_tx_interface->length = dma_tx_len*sizeof(Dma_tx_t);
    }

    void Device::Init_DMA_BS(std::string dma_path)
    {
        dma_bs_fd = open(dma_path.c_str(), O_RDWR);
        if (dma_bs_fd < 1){
            throw Error("can't open TX_BS loop device\n");
        }

        dma_bs_interface = (struct dma_proxy_channel_interface *)mmap(NULL, sizeof(struct dma_proxy_channel_interface),
                                        PROT_READ | PROT_WRITE, MAP_SHARED, dma_bs_fd, 0);
        if (dma_bs_interface == MAP_FAILED){
            throw Error("Failed to mmap\n");
        }
        dma_bs_interface->length = dma_tx_len*sizeof(Dma_tx_t);
    }

    void Device::Start_DMA_RX()
    {
        ptr_dma = 0;
        ptr_read = 0;
        Device_reset();
        ioctl(dma_rx_fd, START_CYCLIC);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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

    void Device::DMA_Transmit(Dma_tx_t *_data)
    {
        memcpy(dma_tx_interface->buffer, _data, dma_tx_len*sizeof(Dma_tx_t));
        ioctl(dma_tx_fd, XFER);
    }

    void Device::DMA_Transmit_BS(Dma_tx_t *_data)
    {
        memcpy(dma_bs_interface->buffer, _data, dma_tx_len*sizeof(Dma_tx_t));
        ioctl(dma_bs_fd, XFER);
    }

    void* Device::Get_DMA_Data()
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
        std::cout << "Time = " << t_result << std::endl;
        return 0;
    }

    int Device::InitBS(const int8_t *_preamble)
    {
        uint32_t pr;
        uint32_t pr_mem[64];
        for(int i = 0; i < 64; i++)
        {
            pr = 0;
            if( i == 63)
                for(int j = 0; j < 31; j++)
                    pr = pr | (_preamble[j+i*32] << (31-j));
            else
                for(int j = 0; j < 32; j++)
                    pr = pr | (_preamble[j+i*32] << (31-j));
            pr_mem[i] = pr;
        }
        uint32_t* mem;
        int fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
        mem = (uint32_t*)mmap(NULL, sizeof(uint32_t)*64, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fd_mem, ADDR_COR_MEM);
        close(fd_mem);
        memcpy(&mem[0], &pr_mem[0], 64*sizeof(uint32_t));

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 0;
    }

    void Device::Device_reset()
    {
        control_struct->control.reset = 0;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        std::cout << "State Reset = 0. Press any key...";
        std::cin.get();
        control_struct->control.reset = 1;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        std::cout << "State Reset = 1.\n";
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


    void *Device::read()
    {
        if (Check_valid_buf(ptr_read, ptr_dma) < 1)
            return nullptr;
        void *ret = &dma_rx_interface->buffer[dma_rx_len*sizeof(Dma_rx_t)*ptr_read];
        ptr_read = (ptr_read + 1) % RX_BUFFER_COUNT;
        return ret;
    }

    Error::Error(const std::string &er) : runtime_error(er) {}
}
