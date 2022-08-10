#include <stdio.h>
#include <csignal>
#include "device.h"
#include "lmk.h"

static volatile std::sig_atomic_t exit_thread = 0;
static volatile std::sig_atomic_t run_dma_tx = 0;
void sigintHandler(int sig)
{
    exit_thread = 1;
}

struct Queue_data
{
    std::vector<d_buffer_t> rec_data;
};
std::queue<Queue_data> queue_tx;
static pthread_mutex_t lock;

void DMA_Transmit(METEOR::Device _dev)
{
    printf("Start DMA_TX\n");
    Queue_data QI;
    uint32_t lost_sample_check = 0;
#if DAC_UDP_SEND_ON
    udp_receiver *u_r = new udp_receiver(UDP_REC_ADDR, UDP_REC_PORT, BUFFER_SIZE*sizeof(d_buffer_t));
#endif
    while(run_dma_tx == 0)
    {
#if !DAC_UDP_SEND_ON
        if (queue_tx.size() > 0)
        {
            pthread_mutex_lock(&lock);
            QI = queue_tx.front();
            queue_tx.pop();
            pthread_mutex_unlock(&lock);

            _dev.DMA_Transmit(QI.rec_data.data());
        }
        else
            usleep(2);
#else
        _dev.DMA_Transmit((d_buffer_t*)u_r->get_data());
#endif
        uint32_t dev_ls = _dev.Get_lost_sample();
        if (lost_sample_check != dev_ls)
        {
            printf("lost_sample %d\n", dev_ls);
            lost_sample_check = dev_ls;
        }
    }
    printf("Stop DMA_TX\n");
#if DAC_UDP_SEND_ON
    delete u_r;
#endif
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, sigintHandler);

    LMK3318::LMK lmk;
    lmk.lmk_init();

    METEOR::Device dev;
    dev.Init_DMA_RX(DEVICE_RX);
#if DAC_SEND
    dev.Init_DMA_TX(DEVICE_TX);
#endif

    dev.Start_DMA_RX();

    d_buffer *tail_buf;
    uint16_t count = 0;
#if UDP_SEND
    UDP::Udp udp(UDP_SEND_ADDR, UDP_SEND_PORT);
#endif

#if FILE_SAVE_10MHz
    std::ofstream file_10MHz("rec_10MHz.cs16", std::ios::app | std::ios::binary);
    const int size = 8192*2;
    d_buffer_t *temp = new d_buffer_t[BUFFER_SIZE*size];
    int c = 0;
#endif

#if FILE_SAVE
    std::ofstream file("rec.cs16", std::ios::app | std::ios::binary);
#endif
    while(exit_thread == 0)
    {
        tail_buf = dev.Get_DMA_Data();
        if(tail_buf)
        {
#if FILE_SAVE
            file.write((char*)tail_buf, BUFFER_SIZE*sizeof(d_buffer_t));
#endif
#if DAC_SEND & !DAC_UDP_SEND_ON
            pthread_mutex_lock(&lock);
            Queue_data q_tx;
            q_tx.rec_data.insert(q_tx.rec_data.end(), tail_buf, &tail_buf[1024]);
            queue_tx.push(q_tx);
            pthread_mutex_unlock(&lock);
#endif
#if COUNTER_TEST_EN
            if ((tail_buf[0].x - count) != BUFFER_SIZE)
                printf("Error count test : %u %u\n", tail_buf[0].x, count);
            count = tail_buf[0].x;
#endif
#if FILE_SAVE_10MHz
            memcpy(&temp[c*BUFFER_SIZE], tail_buf, BUFFER_SIZE*sizeof(d_buffer_t));
            c++;
            if (c == size)
                continue;
#endif
#if UDP_SEND
            udp.send_data(tail_buf, BUFFER_SIZE);
            usleep(1);
#endif
        }
    }

#if FILE_SAVE_10MHz
    for(int i = 0; i < size; i++)
    {
        file_10MHz.write((char*)(&temp[i*BUFFER_SIZE]), BUFFER_SIZE*sizeof(d_buffer_t));
    }
    file_10MHz.close();
#endif

    dev.Stop_DMA_RX();
#if DAC_SEND
    dev.Stop_DMA_TX();
#endif
#if FILE_SAVE
    file.close();
#endif

    return 0;
}
