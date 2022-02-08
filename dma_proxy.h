#ifndef DMA_PROXY_H
#define DMA_PROXY_H

#define NUM_CH                  1
#define IQ_WEIGHT               2
#define TYPE_SIZE               2

#define PART_BUF_NUM            8
#define BUF_NUM             	32
#define TRANSACTION_SIZE        1024

#define REC_BUF_SIZE (TRANSACTION_SIZE*NUM_CH*IQ_WEIGHT)
#define ALL_BUF_SIZE (TRANSACTION_SIZE*NUM_CH*IQ_WEIGHT*PART_BUF_NUM)

enum PROXY_DMA_REQUEST
{
    PROXY_DMA_START 	= 0,
    PROXY_DMA_STOP      = 1,
    PROXY_DMA_TRANSFER 	= 3,
    PROXY_DMA_GET_DATA  = 5
};

enum PROXY_DMA_STATUS
{
    PROXY_NO_ERROR 	= 0,
    PROXY_BUSY 		= 1,
    PROXY_TIMEOUT 	= 2,
    PROXY_ERROR 	= 3
};

struct dma_proxy_channel_interface
{
    short buffer[REC_BUF_SIZE];
    short buffer_data[ALL_BUF_SIZE*BUF_NUM];
    unsigned int length;
    unsigned int status;
}__attribute__ ((aligned (1024)));

#endif // DMA_PROXY_H
