#include <stdio.h>
#include <csignal>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "bs_preamble.h"
#include "device.h"
#include "lmk.h"
#include "command_arguments.h"

static volatile std::sig_atomic_t exit_thread = 0;
static volatile std::sig_atomic_t run_dma_tx = 0;
static volatile std::sig_atomic_t run_check_cf = 0;

std::queue<std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len>> queue_tx;
std::mutex lock_tx;
std::condition_variable cv_tx;

std::queue<std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len>> queue_cf;
std::mutex lock_cf;
std::condition_variable cv_cf;

std::queue<std::array<METEOR::Dma_tx_t, METEOR::dma_tx_len*BUFF_DMA_COUNT>> queue_udp_rec;
std::mutex lock_udp_rec;
std::condition_variable cv_udp_rec;

void sigintHandler(int sig)
{
    exit_thread = 1;
    run_dma_tx = 1;
    run_check_cf = 1;
}

/**
 * @brief UDP_Rec       receiving data via udp for further sending to dma
 * @param ca
 * @param _dam_tx_len
 */
void UDP_Rec(const CommandArgs &ca, int _dam_tx_len)
{
    udp_receiver u_r(ca.udp.addr_rx_udp, ca.udp.port_rx_udp, _dam_tx_len*BUFF_DMA_COUNT);
    METEOR::Dma_tx_t *data = (METEOR::Dma_tx_t*)u_r.get_data();

    std::array<METEOR::Dma_tx_t, METEOR::dma_tx_len*BUFF_DMA_COUNT> q_rec;
    std::array<METEOR::Dma_tx_t, METEOR::dma_tx_len*BUFF_DMA_COUNT>::iterator ibegin = q_rec.begin();
    while(run_dma_tx == 0)
    {
        if (u_r.available_data() > 0)
        {
            std::unique_lock<std::mutex> lock(lock_udp_rec);
            if (queue_udp_rec.size() >= 512)
            {
                std::queue<std::array<METEOR::Dma_tx_t, METEOR::dma_tx_len*BUFF_DMA_COUNT>> zero;
                queue_udp_rec.swap(zero);
                std::cout << "UDP REC Buffer overflow...\n";
            }
            std::copy(data, &(data)[_dam_tx_len*BUFF_DMA_COUNT], ibegin);
            queue_udp_rec.push(q_rec);
            cv_udp_rec.notify_one();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    }
}

/**
 * @brief DMA_Transmit  data transfer in RF
 * @param _dev
 * @param udp_tx_en
 * @param _dam_tx_len
 */
void DMA_Transmit(METEOR::Device &_dev, const bool udp_tx_en, int _dam_tx_len)
{
    std::cout << "Start DMA_TX\n";
    uint32_t lost_sample_check = 0;
    std::array<METEOR::Dma_tx_t, METEOR::dma_tx_len*BUFF_DMA_COUNT> QI_udp;
    std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len> QI_tx;
    std::array<METEOR::Dma_tx_t, METEOR::dma_tx_len> send_dma;
    while(run_dma_tx == 0)
    {
        if(udp_tx_en)
        {
            std::unique_lock<std::mutex> lock(lock_udp_rec);
            cv_udp_rec.wait(lock, []{return !queue_udp_rec.empty() | run_dma_tx;});

            if(!queue_udp_rec.empty())
            {
                QI_udp = std::move(queue_udp_rec.front());
                queue_udp_rec.pop();

                for(int i = 0; i < BUFF_DMA_COUNT; i++)
                    _dev.DMA_Transmit(&(QI_udp[i*_dam_tx_len]));
            }
        }
        else
        {
            std::unique_lock<std::mutex> lock(lock_tx);
            cv_tx.wait(lock, []{return !queue_tx.empty() | run_dma_tx;});

            if(!queue_tx.empty())
            {
                QI_tx = std::move(queue_tx.front());
                queue_tx.pop();

                if(queue_tx.size() >= 512)
                {
                    std::queue<std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len>> zero;
                    queue_tx.swap(zero);
                    std::cout << "Queue queue_tx size exceeded. Cleaning done" << std::endl;
                }
                lock_tx.unlock();

                for(int i = 0; i < METEOR::dma_rx_len; i++)
                {
                    send_dma[i].cn.real(QI_tx[i].cn.real());
                    send_dma[i].cn.imag(QI_tx[i].cn.imag());
                }
                _dev.DMA_Transmit(send_dma.data());
            }
        }

        uint32_t dev_ls = _dev.Get_lost_sample();
        if (lost_sample_check != dev_ls)
        {
            std::cout << "lost_sample " << lost_sample_check << "\r";
            lost_sample_check = dev_ls;
        }
    }
    std::cout << "Stop DMA_TX\n";
}

/**
 * @brief CheckCF   determination of correlation function threshold exceeding
 * @param _th       threshold
 */
void CheckCF(float _th)
{
    std::cout << "Start CheckCF\n";
    int done = 0;
    std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len> QI;
    time_t t;
    while(run_check_cf == 0)
    {
        std::unique_lock<std::mutex> lock(lock_cf);
        cv_cf.wait(lock, []{return !queue_cf.empty() | run_check_cf;});

        if(!queue_cf.empty())
        {
            QI = std::move(queue_cf.front());
            int size = queue_cf.size();
            queue_cf.pop();

            done = 0;
            if(size >= 512)
            {
                std::queue<std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len>> zero;
                queue_cf.swap(zero);
                std::cout << "Queue queue_cf size exceeded. Cleaning done" << std::endl;
            }

            for(auto &i : QI)
            {
                if (i.cor > _th)
                {
                    if (done == 0)
                    {
                        std::cout << "\033[1;36m";
                        t = time(NULL);
                        std::cout << "Time : " << ctime(&t);
                        std::cout << "\033[0m";
                    }
                    std::cout << "CF value = " << std::setprecision(6) << i.cor << std::endl;
                    done = 1;
                }
            }
            if(done)
                std::cout << "\n\n";
        }
    }
    std::queue<std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len>> zero;
    queue_cf.swap(zero);
    std::cout << "Stop CheckCF\n";
}

/**
 * @brief DMA_Transmit_BS Only test mode
 * @param _dev
 */
void DMA_Transmit_BS(METEOR::Device &_dev)
{
    std::ifstream file("meteor_2047.cs16", std::ios::app | std::ios::binary);
    file.seekg(0, std::ios::end);
    float fsize = file.tellg();
    file.seekg(0, std::ios::beg);
    if(fsize == 0)
    {
        std::cout << "File size = 0. Close" << std::endl;
        sigintHandler(0);
        return;
    }
    std::cout << "file size = " << std::setprecision(2) << fsize / 1024 / 1024 << "MB\n";
    std::array<METEOR::Dma_tx_t, METEOR::dma_tx_len> data_file;

    int count = 0;
    float file_size = fsize;
    constexpr int size_read = METEOR::dma_tx_len*sizeof(METEOR::Dma_tx_t);
    while(run_dma_tx == 0)
    {
#if 1
        if (file_size < size_read)
        {
            std::memset(data_file.begin(), 0, METEOR::dma_tx_len);
            file.read((char*)data_file.begin(), file_size/sizeof(METEOR::Dma_tx_t));
        }
        else
            file.read((char*)data_file.begin(), size_read);

        _dev.DMA_Transmit_BS(data_file.begin());

        file_size -= size_read;
        if(file_size <= 0)
        {
            file.seekg(0, std::ios::beg);
            file_size = fsize;
        }
#else
        for(int i = 0; i < 8192; i++)
        {
            if(count == 89)
                count = 1;
            else
                count++;
            data_file[i].data.real(count);
            data_file[i].data.imag(count);

        }
        _dev.DMA_Transmit_BS(data_file.begin());
#endif
    }
    data_file.~array();
    file.close();
    std::cout << "Stop DMA_BS\n";
}

int main(int argc, char* argv[])
{
    std::signal(SIGINT, sigintHandler);

    const CommandArgs ca(argc, argv);
    ca.PringArgs();

    LMK3318::LMK lmk;
    lmk.lmk_init();

    METEOR::Device dev;
    dev.Init();

    dev.Set_Param(ca.base.sample_rate,
                  ca.base.counter_test_en,
                  ca.cor.cor_mode_file_test,
                  ca.adc.adc_debug_en,
                  ca.adc.adc_freq,
                  ca.adc.adc_debug_freq,
                  ca.adc.adc_sys_clk,
                  ca.dac.dac_debug_en,
                  ca.dac.dac_freq,
                  ca.dac.dac_sys_clk);

    if (ca.cor.cor_mode_en)
        dev.InitBS(bs_preamble);
    dev.Init_DMA_RX(DEVICE_RX);
    dev.Start_DMA_RX();
    dev.Init_DMA_BS(DEVICE_BS);

    std::thread dma_tx;
    std::thread udp_rec;
    if(ca.dac.dac_tx_en)
    {
        dev.Init_DMA_TX(DEVICE_TX);
        dma_tx =  std::thread(DMA_Transmit, std::ref(dev), ca.dac.dac_tx_udp_en, METEOR::dma_tx_len);
        if(ca.dac.dac_tx_udp_en)
            udp_rec =  std::thread(UDP_Rec, ca, METEOR::dma_tx_len);
    }

    METEOR::Dma_rx_t *tail_buf;
    UDP::Udp udp;
    std::array<std::complex<int16_t>, METEOR::dma_rx_len> udp_data;
    if (ca.udp.udp_tx_en)
        udp.Init(ca.udp.addr_tx_udp, ca.udp.port_tx_udp);

    std::ofstream file;
    if (ca.base.file_save_en)
        file.open("rec.cs16", std::ios::app | std::ios::binary);

    std::thread dma_bs;
    if (ca.cor.cor_mode_file_test)
        dma_bs = std::thread(DMA_Transmit_BS, std::ref(dev));

    std::thread check_cf;
    if (ca.cor.cor_mode_en)
        check_cf = std::thread(CheckCF, ca.cor.th);

    std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len> q_dump;
    std::array<METEOR::Dma_rx_t, METEOR::dma_rx_len>::iterator ibegin = q_dump.begin();
    while(exit_thread == 0)
    {
        tail_buf = (METEOR::Dma_rx_t*)dev.Get_DMA_Data();
        if(tail_buf != nullptr)
        {
            if (ca.cor.cor_mode_en)
            {
                {
                    std::unique_lock<std::mutex> lock(lock_cf);
                    std::copy(tail_buf, &tail_buf[METEOR::dma_rx_len], ibegin);
                    queue_cf.push(q_dump);
                }
                cv_cf.notify_one();
            }

            if (ca.base.file_save_en)
                file.write((char*)tail_buf, METEOR::dma_rx_len*sizeof(METEOR::Dma_rx_t));

            if(ca.dac.dac_tx_en & !ca.dac.dac_tx_udp_en)
            {
                {
                    std::unique_lock<std::mutex> lock(lock_tx);
                    std::copy(tail_buf, &tail_buf[METEOR::dma_rx_len], ibegin);
                    queue_tx.push(q_dump);
                }
                cv_tx.notify_one();
            }

            if (ca.udp.udp_tx_en)
            {
                for(int i = 0; i < METEOR::dma_rx_len; i++)
                {
                    udp_data[i].real(tail_buf[i].cn.real());
                    udp_data[i].imag(tail_buf[i].cn.imag());
                }
                udp.send_data(udp_data.begin(), METEOR::dma_rx_len);
            }
        }
    }

    cv_tx.notify_one();
    cv_cf.notify_one();
    cv_udp_rec.notify_one();

    if (ca.udp.udp_tx_en)
        udp_data.~array();

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
        std::cout << "sync() wait...\n";
        sync();
        file.close();
    }

    if (ca.cor.cor_mode_en)
        check_cf.join();
    if (ca.cor.cor_mode_file_test)
        dma_bs.join();

    return 0;
}
