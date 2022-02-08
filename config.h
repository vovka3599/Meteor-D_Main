#ifndef CONFIG_H
#define CONFIG_H
#include <iostream>
#include "register.h"

// UDP config
#define ADDRESS_DEFAULT     "192.168.1.193"
#define PORT_DEFAULT        1234
#define FFT_SIZE_DEFAULT    1024*8

// Registers control
#define ADDR_REGISTERS      0x43C00000L

// Control config
#define FILE_SAVE           0
#define UDP_SEND            1
#define TIME_ON             0
#define DAC_SEND            1

// DMA config
#define DEVICE_RX           "/dev/dma_ltc_rx"
#define DEVICE_TX           "/dev/dma_max_tx"

// Decimation config
const uint8_t SAMPLE_FREQ               = SAMP_FREQ_1_MHz;

// DDS ADC config
const uint32_t ADC_DEBUG_ON             = 0;
const double ADC_DDS_OUT_FREQ_MHz       = 21.000;
const double ADC_DDS_SYSTEM_CLOCK_MHz   = 80.000;
const uint32_t ADC_DDS_VALUE            = (ADC_DDS_OUT_FREQ_MHz/ADC_DDS_SYSTEM_CLOCK_MHz)*(1<<27);

// DDS DAC config
const double DAC_DDS_OUT_FREQ_MHz       = 59.000;
const double DAC_DDS_SYSTEM_CLOCK_MHz   = 320.000;
const uint32_t DAC_DDS_VALUE            = (DAC_DDS_OUT_FREQ_MHz / DAC_DDS_SYSTEM_CLOCK_MHz) * (1<<29);
const bool DAC_REAL_DATA                = 1;

// DDS ADC DEBUG config
const double ADC_DDS_DEBUG_OUT_FREQ_MHz = 59.000;
const uint32_t ADC_DDS_DEBUG_VALUE      = (ADC_DDS_DEBUG_OUT_FREQ_MHz/ADC_DDS_SYSTEM_CLOCK_MHz)*(1<<27);

#endif // CONFIG_H
