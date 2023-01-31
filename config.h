#ifndef CONFIG_H
#define CONFIG_H
#include <iostream>
#include "register.h"

// DMA config
#define DEVICE_RX           "/dev/dma_ltc_rx"
#define DEVICE_TX           "/dev/dma_max_tx"

// Registers control
#define ADDR_REGISTERS      0x43C00000L

// Control config
#define BUFF_DAM_COUNT      4

#endif // CONFIG_H
