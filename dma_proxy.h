#ifndef DMA_PROXY_H
#define DMA_PROXY_H

#define BUFFER_SIZE 64*65536                /* must match driver exactly */
#define BUFFER_COUNT 1						/* driver only */

#define TX_BUFFER_COUNT 	1
#define RX_BUFFER_COUNT 	8
#define BUFFER_INCREMENT	1				/* normally 1, but skipping buffers (2) defeats prefetching in the CPU */

#define START_CYCLIC 			_IOW('a','a',int32_t*)
#define STOP_CYCLIC 			_IOW('a','b',int32_t*)
#define PROXY_DMA_SET_SIGNAL	_IOW('a','c',int32_t*)
#define FINISH_XFER 			_IOW('a','d',int32_t*)
#define START_XFER 				_IOW('a','e',int32_t*)
#define XFER 					_IOR('a','f',int32_t*)

typedef struct signal_parameters{
    unsigned int on;
    unsigned int data;
    unsigned int period;
} signal_parameters;

struct dma_proxy_channel_interface {
    signed char buffer[BUFFER_SIZE*RX_BUFFER_COUNT];
    enum proxy_status { PROXY_NO_ERROR = 0, PROXY_BUSY = 1, PROXY_TIMEOUT = 2, PROXY_ERROR = 3 } status;
    unsigned int length;
} __attribute__ ((aligned (1024)));		/* 64 byte alignment required for DMA, but 1024 handy for viewing memory */

#endif // DMA_PROXY_H
