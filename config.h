#ifndef CONFIG_H
#define CONFIG_H
#include <iostream>
#include "register.h"

// UDP SEND config
#define UDP_SEND_ADDR       "192.168.1.193"
#define UDP_SEND_PORT       1234

// UDP REC config
#define UDP_REC_ADDR        "192.168.1.170"
#define UDP_REC_PORT        4321

// Registers control
#define ADDR_REGISTERS      0x43C00000L

// Control config
#define FILE_SAVE           0
#define UDP_SEND            1
#define TIME_ON             0
#define DAC_SEND            0
#define DAC_UDP_SEND_ON     0
#define COUNTER_TEST_EN     0   // You need to change the buffer type to "unsigned short" in dma_proxy.h
#define FILE_SAVE_10MHz     0   // Disable all other operations for this mode

// DMA config
#define DEVICE_RX           "/dev/dma_ltc_rx"
#define DEVICE_TX           "/dev/dma_max_tx"

// Decimation config
const uint8_t SAMPLE_FREQ               = SAMP_FREQ_10_MHz;

// DDS ADC config
const double ADC_DDS_OUT_FREQ_MHz       = 21.000;
const double ADC_DDS_SYSTEM_CLOCK_MHz   = 80.000;
const uint32_t ADC_DDS_VALUE            = (ADC_DDS_OUT_FREQ_MHz/ADC_DDS_SYSTEM_CLOCK_MHz)*(1<<27);

// DDS DAC config
const double DAC_DDS_OUT_FREQ_MHz       = 59.000;
const double DAC_DDS_SYSTEM_CLOCK_MHz   = 320.000;
const uint32_t DAC_DDS_VALUE            = (DAC_DDS_OUT_FREQ_MHz / DAC_DDS_SYSTEM_CLOCK_MHz) * (1<<29);
const bool DAC_REAL_DATA                = 1;

// DDS ADC DEBUG config
const uint32_t ADC_DEBUG_ON             = 1;
const double ADC_DDS_DEBUG_OUT_FREQ_MHz = 59.001;
const uint32_t ADC_DDS_DEBUG_VALUE      = (ADC_DDS_DEBUG_OUT_FREQ_MHz/ADC_DDS_SYSTEM_CLOCK_MHz)*(1<<27);

#endif // CONFIG_H
