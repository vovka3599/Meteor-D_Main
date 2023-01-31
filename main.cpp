#include <stdio.h>
#include <csignal>
#include "device.h"
#include "lmk.h"
#include "command_arguments.h"

static volatile std::sig_atomic_t exit_thread = 0;
static volatile std::sig_atomic_t run_dma_tx = 0;

void sigintHandler(int sig)
{
    exit_thread = 1;
    run_dma_tx = 1;
}

struct Queue_data
{
    std::vector<d_buffer_t> rec_data;
};
std::queue<Queue_data> queue_tx;
static pthread_mutex_t lock_tx;

std::queue<Queue_data> queue_udp_rec;
static pthread_mutex_t lock_udp_rec;

void UDP_Rec(const CommandArgs &ca)
{
    udp_receiver *u_r = new udp_receiver(ca.udp.addr_rx_udp, ca.udp.port_rx_udp, BUFFER_SIZE*BUFF_DAM_COUNT);
    d_buffer_t *data = (d_buffer_t*)u_r->get_data();
    while(run_dma_tx == 0)
    {
        if (u_r->available_data() > 0)
        {
            pthread_mutex_trylock(&lock_udp_rec);
            Queue_data q_udp;
            q_udp.rec_data.insert(q_udp.rec_data.end(), data, &(data)[BUFFER_SIZE*BUFF_DAM_COUNT]);
            if (queue_udp_rec.size() >= 512)
            {
                std::queue<Queue_data> tmp;
                queue_udp_rec.swap(tmp);
                printf("UDP REC Buffer overflow...\n");
            }
            queue_udp_rec.push(q_udp);
            pthread_mutex_unlock(&lock_udp_rec);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    }
    delete u_r;
}

void DMA_Transmit(METEOR::Device _dev, const bool udp_tx_en)
{
    printf("Start DMA_TX\n");
    Queue_data QI;
    uint32_t lost_sample_check = 0;
    while(run_dma_tx == 0)
    {
        if(udp_tx_en)
        {
            if (queue_udp_rec.size() > 0)
            {
                pthread_mutex_trylock(&lock_udp_rec);
                QI = queue_udp_rec.front();
                queue_udp_rec.pop();
                pthread_mutex_unlock(&lock_udp_rec);

                for(int i = 0; i < BUFF_DAM_COUNT; i++)
                {
                    _dev.DMA_Transmit(&(QI.rec_data.data()[i*BUFFER_SIZE]));
                }
            }
        }
        else
        {
            if (queue_tx.size() > 0)
            {
                pthread_mutex_trylock(&lock_tx);
                QI = queue_tx.front();
                queue_tx.pop();
                pthread_mutex_unlock(&lock_tx);

                _dev.DMA_Transmit(QI.rec_data.data());
            }
        }

        uint32_t dev_ls = _dev.Get_lost_sample();
        if (lost_sample_check != dev_ls)
        {
            printf("lost_sample %u", dev_ls);
            printf("\r");
            lost_sample_check = dev_ls;
        }
    }
    printf("Stop DMA_TX\n");
}

int main(int argc, char* argv[])
{
    const CommandArgs ca(argc, argv);
    ca.PringArgs();

    std::signal(SIGINT, sigintHandler);

    LMK3318::LMK lmk;
    lmk.lmk_init();

    METEOR::Device dev;
    dev.Init();

    dev.Set_Param(ca.base.sample_rate,
                  ca.base.counter_test_en,
                  ca.adc.adc_debug_en,
                  ca.adc.adc_freq,
                  ca.adc.adc_debug_freq,
                  ca.adc.adc_sys_clk,
                  ca.dac.dac_debug_en,
                  ca.dac.dac_freq,
                  ca.dac.dac_sys_clk);

    dev.Init_DMA_RX(DEVICE_RX);
    dev.Start_DMA_RX();

    std::thread dma_tx;
    std::thread udp_rec;
    if(ca.dac.dac_tx_en)
    {
        dev.Init_DMA_TX(DEVICE_TX);
        dma_tx =  std::thread(DMA_Transmit, dev, ca.dac.dac_tx_udp_en);
        if(ca.dac.dac_tx_udp_en)
            udp_rec =  std::thread(UDP_Rec, ca);
    }

    d_buffer *tail_buf;
    UDP::Udp udp;
    if (ca.udp.udp_tx_en)
        udp.Init(ca.udp.addr_tx_udp, ca.udp.port_tx_udp);

    std::ofstream file;
    if (ca.base.file_save_en)
        file.open("rec.cs16", std::ios::app | std::ios::binary);

    while(exit_thread == 0)
    {
        tail_buf = dev.Get_DMA_Data();
        if(tail_buf)
        {
            if (ca.base.file_save_en)
                file.write((char*)tail_buf, BUFFER_SIZE*sizeof(d_buffer_t));
            if(ca.dac.dac_tx_en & !ca.dac.dac_tx_udp_en)
            {
                pthread_mutex_trylock(&lock_tx);
                Queue_data q_dump;
                q_dump.rec_data.insert(q_dump.rec_data.end(), tail_buf, &tail_buf[BUFFER_SIZE]);
                queue_tx.push(q_dump);
                pthread_mutex_unlock(&lock_tx);
            }
            if (ca.udp.udp_tx_en)
                udp.send_data(tail_buf, BUFFER_SIZE);
        }
    }

    if(ca.dac.dac_tx_en)
    {
        dma_tx.join();
        if(ca.dac.dac_tx_udp_en)
            udp_rec.join();
        dev.Stop_DMA_TX();
    }

    dev.Stop_DMA_RX();
    lmk.lmk_stop();

    if (ca.base.file_save_en)
    {
        file.flush();
        printf("sync() wait...\n");
        sync();
        file.close();
    }

    return 0;
}
