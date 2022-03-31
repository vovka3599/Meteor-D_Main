#include "device.h"

// ================= PUBLIC =================
Device::Device()
{

}

void Device::Init(std::string _dma_rx_path, std::string _dma_tx_path, int _udp_socket)
{
    udp_socket = _udp_socket;
    Init_DMA_RX(_dma_rx_path);
    Init_control_struct();
#if DAC_SEND
    Init_DMA_TX(_dma_tx_path);
    pthread_mutex_init(&lock, NULL);
#endif
}

void Device::Start_DMA_RX()
{
    int tmp;
    ioctl(dma_rx_fd, PROXY_DMA_STOP, &tmp);
    Device_reset();
    ioctl(dma_rx_fd, PROXY_DMA_STOP, &tmp);
    ioctl(dma_rx_fd, PROXY_DMA_START, &tmp);

    run_dma_rx = 1;
    dma_rx = new std::thread(&Device::DMA_Receive, this);
    usleep(1000);
}

void Device::Stop_DMA_RX()
{
    run_dma_rx = 0;
    dma_rx->join();

    munmap(dma_rx_interface, sizeof(struct dma_proxy_channel_interface));
    close(dma_rx_fd);
}

void Device::Start_DMA_TX()
{
    run_dma_tx = 1;
    dma_tx = new std::thread(&Device::DMA_Transmit, this);
}

void Device::Stop_DMA_TX()
{
    run_dma_tx = 0;
    dma_tx->join();

    munmap(dma_tx_interface, sizeof(struct dma_proxy_channel_interface));
    close(dma_tx_fd);
}

void Device::Set_control_struct()
{
    control_struct->control.reset = 1;

    control_struct->control.adc_dith = 0;
    control_struct->control.adc_pga = 0;
    control_struct->control.adc_rand = 0;
    control_struct->control.adc_shdn = 0;

    control_struct->control.adc_debug = ADC_DEBUG_ON;

    control_struct->control.samp_freq = SAMPLE_FREQ;

    control_struct->control.dac_real_data = DAC_REAL_DATA;
    control_struct->dac_dds_freq = DAC_DDS_VALUE;

    control_struct->adc_dds_freq = ADC_DDS_VALUE;

    control_struct->adc_dds_freq_debug = ADC_DDS_DEBUG_VALUE;

    printf("adc_dds_freq = %x\n", ADC_DDS_VALUE);
    printf("dac_dds_freq = %x\n", DAC_DDS_VALUE);
}
// ================= PUBLIC =================

// ================= PRIVATE =================
void Device::Device_reset()
{
    control_struct->control.reset = 0;
    usleep(10000);
    control_struct->control.reset = 1;
    usleep(10000);
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
    dma_rx_interface->length = TRANSACTION_SIZE*NUM_CH*IQ_WEIGHT*TYPE_SIZE;
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
    dma_tx_interface->length = TRANSACTION_SIZE*NUM_CH*IQ_WEIGHT*TYPE_SIZE;
}

void Device::Init_control_struct()
{
    uint32_t* status_reg;
    int fd_mem = open("/dev/mem", O_RDWR | O_SYNC);
    status_reg = (uint32_t*)mmap(NULL, sizeof(Registers), PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, ADDR_REGISTERS);
    control_struct = (struct Registers*)status_reg;
    close(fd_mem);
}

void Device::Time_start()
{
    t_start = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void Device::Time_end()
{
    t_end = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    t_result = t_end - t_start;
    printf("time = %d\n", (int)t_result);
}

void Device::DMA_Transmit()
{
    printf("Start DMA_TX\n");
    int tmp;
    Queue_data QI;
    uint32_t lost_sample_check = 0;
#if DAC_UDP_SEND_ON
    udp_receiver *u_r = new udp_receiver(ADDR_UDP_REC, PORT_UDP_REC, TRANSACTION_SIZE*sizeof(l_complex<uint16_t>));
#endif

    while(run_dma_tx)
    {
#if !DAC_UDP_SEND_ON
        if (queue_tx.size() > 0)
        {
            pthread_mutex_lock(&lock);
            QI = queue_tx.front();
            queue_tx.pop();
            pthread_mutex_unlock(&lock);

            memcpy(dma_tx_interface->buffer, QI.rec_data.data(), TRANSACTION_SIZE*sizeof(l_complex<int16_t>));
            ioctl(dma_tx_fd, PROXY_DMA_TRANSFER, &tmp);
        }
        else
            usleep(2);
#else
        memcpy(dma_tx_interface->buffer, u_r->get_data(), TRANSACTION_SIZE*sizeof(l_complex<int16_t>));
        ioctl(dma_tx_fd, PROXY_DMA_TRANSFER, &tmp);
#endif
        if (lost_sample_check != control_struct->lost_sample)
        {
            printf("lost_sample %d\n", control_struct->lost_sample);
            lost_sample_check = control_struct->lost_sample;
        }
    }
#if DAC_UDP_SEND_ON
    delete u_r;
#endif
    printf("Stop DMA_TX\n");
}

void Device::DMA_Receive()
{
    int tmp = 0;
    printf("Start DMA_RX\n");
#if FILE_SAVE
    std::ofstream file("rec_data.complex", std::ios::app | std::ios::binary);
#endif
    l_complex<int16_t> *rec_buf = new l_complex<int16_t>[TRANSACTION_SIZE];
 #if UDP_SEND
    l_complex<float> *udp_buf = new l_complex<float>[TRANSACTION_SIZE];
#endif
    while(run_dma_rx)
    {
            tail_buf = ioctl(dma_rx_fd, PROXY_DMA_GET_DATA, &tmp);
            if (tail_buf == -1)
            {
                printf("DMA read error. Exit\n");
                exit(0);
            }

//            int temp = open("/sys/bus/iio/devices/iio:device0/in_temp0_raw", O_RDONLY);
//            if (tail_buf % 32 == 0)
//            {
//                char temp_read[4];
//                read(temp, &temp_read, 4);
//                lseek(temp, 0, SEEK_SET);

//                int temp_pr = std::atoi(temp_read);
//                float rez_temp = ((temp_pr-2219)*123.0407)/1000.0;
//                printf("temperature  = %.2f\n", rez_temp);
//            }
//            close(temp);

#if TIME_ON
            Time_start();
#endif
#if FILE_SAVE
            file.write((char*)(dma_rx_interface->buffer_data + tail_buf*ALL_BUF_SIZE), TRANSACTION_SIZE*PART_BUF_NUM*sizeof(l_int16_t));
#endif
            for(int j = 0; j < PART_BUF_NUM; j++)
            {
                memcpy(rec_buf, dma_rx_interface->buffer_data+tail_buf*ALL_BUF_SIZE+j*TRANSACTION_SIZE*NUM_CH*IQ_WEIGHT, TRANSACTION_SIZE*sizeof(l_complex<int16_t>));
#if DAC_SEND && !DAC_UDP_SEND_ON
                pthread_mutex_lock(&lock);
                Queue_data q_tx;
                q_tx.rec_data.insert(q_tx.rec_data.end(), rec_buf, rec_buf + TRANSACTION_SIZE);
                queue_tx.push(q_tx);
                pthread_mutex_unlock(&lock);
#endif
#if UDP_SEND
                for(int i = 0; i < TRANSACTION_SIZE; i++)
                {
                    udp_buf[i].x = rec_buf[i].x;
                    udp_buf[i].y = rec_buf[i].y;
                }

                ssize_t ret = send(udp_socket, udp_buf, TRANSACTION_SIZE*sizeof(l_complex<float>), 0);
                if(ret != TRANSACTION_SIZE*sizeof(l_complex<float>))
                {
                    printf("Error send data to UDP. Expected %d, send %d\n", TRANSACTION_SIZE*sizeof(l_complex<float>), (int)ret);
                    if(ret < 0)
                        printf("Error send UDP: %s\n", strerror(errno));
                }
                //usleep(2900);
#endif
            }
#if TIME_ON
            Time_end();
#endif
    }
#if FILE_SAVE
    file.close();
#endif
    delete[] rec_buf;
#if UDP_SEND
    delete[] udp_buf;
#endif
    printf("Stop DMA_RX\n");
}
// ================= PRIVATE =================
