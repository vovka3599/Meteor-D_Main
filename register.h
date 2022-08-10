#ifndef REGISTER_H
#define REGISTER_H

#include <iostream>

enum
{
    SAMP_FREQ_10_MHz    = 0,
    SAMP_FREQ_1_MHz     = 1,
    SAMP_FREQ_500_kHz   = 2,
    SAMP_FREQ_250_kHz   = 3
};

/**
 * @brief The Registers struct
 */
struct Registers
{
    struct Control
    {
        uint32_t reset          : 1;
        uint32_t samp_freq      : 2;
        uint32_t adc_shdn       : 1;
        uint32_t adc_dith       : 1;
        uint32_t adc_pga        : 1;
        uint32_t adc_rand       : 1;
        uint32_t adc_debug      : 1;
        uint32_t dac_real_data  : 1;
        uint32_t counter_test   : 1;
        uint32_t                : 22;
    };

    volatile Control control;

    volatile uint32_t dac_dds_freq;

    volatile uint32_t adc_dds_freq;
    volatile uint32_t adc_dds_freq_debug;

    volatile uint32_t lost_sample;
};

#endif // REGISTER_H
