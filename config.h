#ifndef CONFIG_H
#define CONFIG_H
#include <iostream>

// DMA config
#define DEVICE_RX           "/dev/dma_ltc_rx"
#define DEVICE_TX           "/dev/dma_max_tx"
#define DEVICE_BS           "/dev/dma_bs_tx"

// Registers control
#define ADDR_REGISTERS      0x83C00000L

// Correlator control
#define ADDR_COR_MEM        0x80000000L

// Control config
#define BUFF_DMA_COUNT      4

#endif // CONFIG_H
