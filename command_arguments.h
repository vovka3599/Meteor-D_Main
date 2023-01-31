#ifndef COMMAND_ARGUMENTS_H
#define COMMAND_ARGUMENTS_H

#pragma once
#include "cxxopts.hpp"
#include <iostream>
#include <iomanip>

struct CommandArgs
{
    CommandArgs(int argc, char **argv)
    {
        cxxopts::Options options("Meteor-D_Main","Meteor-D control program");
        options.add_options("BASE")
                ("counter_test_en", "Replaces data from ADC with unsigned counter. Only flag")
                ("file_save_en", "Writing data from the ADC to a file. Only flag")
                ("s, sample_rate", "Set sample rate:\n0 - SR_10_MHz\n1 - SR_1_MHz\n2 - SR_500_kHz\n3 - SR_250_kHz", cxxopts::value<uint16_t>()->default_value("1"))
                ("h, help", "Print help");
        options.add_options("UDP")
                ("udp_tx_en", "Enable UDP send. Only flag")
                ("a, addr_tx_udp", "IP address to UDP send", cxxopts::value<std::string>()->default_value("192.168.1.193"))
                ("p, port_tx_udp", "Port to UDP send", cxxopts::value<uint16_t>()->default_value("1234"))
                ("udp_rx_en", "Enable UDP recive. Only flag")
                ("A, addr_rx_udp", "IP address to UDP recive", cxxopts::value<std::string>()->default_value("192.168.1.172"))
                ("P, port_rx_udp", "Port to UDP recive", cxxopts::value<uint16_t>()->default_value("4321"));
        options.add_options("ADC")
                ("adc_debug_en", "Replaces data from ADC with generated sine. Only flag")
                ("adc_freq", "RF input frequency in Hz", cxxopts::value<double>()->default_value("59000000"))
                ("adc_debug_freq", "RF input frequency in Hz", cxxopts::value<double>()->default_value("59010000"))
                ("adc_sys_clk", "Clk ADC in Hz. Depends on LMK configuration", cxxopts::value<double>()->default_value("80000000"));
        options.add_options("DAC")
                ("dac_debug_en", "Replaces the real data for the DAC with the generated sine. The dac_tx_en flag is not required. Only flag")
                ("dac_tx_en", "Enabling DAC transmission. Forming a loop with the ADC. Only flag")
                ("dac_tx_udp_en", "Changes the DAC data source to UDP. Requires dac_tx_en and udp_rx_en flag. Only flag")
                ("dac_freq", "RF output frequency in Hz", cxxopts::value<double>()->default_value("59000000"))
                ("dac_sys_clk", "Clk DAC in Hz. Depends on LMK configuration", cxxopts::value<double>()->default_value("320000000"));

        auto result = options.parse(argc, argv);
        if(result.count("h") > 0)
        {
            std::cout << options.help({"BASE", "UDP", "ADC", "DAC"}) << std::endl;
            exit(0);
        }

        if(result.count("dac_tx_udp_en") > 0 && result.count("dac_tx_en") == 0 && result.count("udp_rx_en") == 0)
        {
            std::cout << "The dac_tx_udp_en flag cannot be set without the dac_tx_en and udp_rx_en flag." << std::endl;
            exit(0);
        }

        if(result.count("adc_sys_clk") > 0)
        {
            if(result["adc_sys_clk"].as<double>() == 0.0)
            {
                std::cout << "The adc_sys_clk cannot be null." << std::endl;
                exit(0);
            }
        }

        if(result.count("dac_sys_clk") > 0)
        {
            if(result["dac_sys_clk"].as<double>() == 0.0)
            {
                std::cout << "The dac_sys_clk cannot be null." << std::endl;
                exit(0);
            }
        }

        base.counter_test_en = result.count("counter_test_en");
        base.file_save_en = result.count("file_save_en");
        base.sample_rate = result["sample_rate"].as<uint16_t>();

        udp.udp_tx_en = result.count("udp_tx_en");
        udp.addr_tx_udp = result["addr_tx_udp"].as<std::string>();
        udp.port_tx_udp = result["port_tx_udp"].as<uint16_t>();

        udp.udp_rx_en = result.count("udp_rx_en");
        udp.addr_rx_udp = result["addr_rx_udp"].as<std::string>();
        udp.port_rx_udp = result["port_rx_udp"].as<uint16_t>();

        adc.adc_debug_en = result.count("adc_debug_en");
        adc.adc_freq = result["adc_freq"].as<double>();
        adc.adc_debug_freq = result["adc_debug_freq"].as<double>();
        adc.adc_sys_clk = result["adc_sys_clk"].as<double>();

        dac.dac_debug_en = result.count("dac_debug_en");
        dac.dac_tx_en = result.count("dac_tx_en");
        dac.dac_tx_udp_en = result.count("dac_tx_udp_en");
        dac.dac_freq = result["dac_freq"].as<double>();
        dac.dac_sys_clk = result["dac_sys_clk"].as<double>();

        if(dac.dac_debug_en)
            dac.dac_tx_en = dac.dac_tx_udp_en = false;
    }

    void PringArgs() const
    {
        std::cout << "Args get" << std::endl;
        std::cout << "  BASE" << std::endl;
        std::cout << "\t counter_test_en : " << base.counter_test_en << std::endl;
        std::cout << "\t file_save_en : " << base.file_save_en << std::endl;
        std::cout << "\t sample_rate : ";
        switch (base.sample_rate)
        {
            case 0:
                std::cout << "SR_10_MHz" << std::endl;
                break;
            case 1:
                std::cout << "SR_1_MHz" << std::endl;
                break;
            case 2:
                std::cout << "SR_500_kHz" << std::endl;
                break;
            case 3:
                std::cout << "SR_250_kHz" << std::endl;
                break;
        }

        std::cout << "  UDP" << std::endl;
        std::cout << "\t udp_tx_en : " << udp.udp_tx_en << std::endl;
        if(udp.udp_tx_en)
        {
            std::cout << "\t addr_tx_udp : " << udp.addr_tx_udp << std::endl;
            std::cout << "\t port_tx_udp : " << udp.port_tx_udp << std::endl;
        }

        std::cout << "\t udp_rx_en : " << udp.udp_rx_en << std::endl;
        if(udp.udp_rx_en)
        {
            std::cout << "\t addr_rx_udp : " << udp.addr_rx_udp << std::endl;
            std::cout << "\t port_rx_udp : " << udp.port_rx_udp << std::endl;
        }

        std::cout << "  ADC" << std::endl;
        std::cout << "\t adc_debug_en : " << adc.adc_debug_en << std::endl;
        std::cout << "\t adc_freq : " << std::setprecision(15) << adc.adc_freq << std::endl;
        std::cout << "\t adc_debug_freq : " << std::setprecision(15) << adc.adc_debug_freq << std::endl;
        std::cout << "\t adc_sys_clk : " << std::setprecision(15) << adc.adc_sys_clk << std::endl;

        std::cout << "  DAC" << std::endl;
        std::cout << "\t dac_debug_en : " << dac.dac_debug_en << std::endl;
        std::cout << "\t dac_tx_en : " << dac.dac_tx_en << std::endl;
        std::cout << "\t dac_tx_udp_en : " << dac.dac_tx_udp_en << std::endl;
        std::cout << "\t dac_freq : " << std::setprecision(15) << dac.dac_freq << std::endl;
        std::cout << "\t dac_sys_clk : " << std::setprecision(15) << dac.dac_sys_clk << std::endl;
    }

    struct Base
    {
        bool counter_test_en;
        bool file_save_en;
        uint16_t  sample_rate;
    };

    struct UDP
    {
        bool udp_tx_en;
        std::string addr_tx_udp;
        uint16_t port_tx_udp;

        bool udp_rx_en;
        std::string addr_rx_udp;
        uint16_t port_rx_udp;
    };

    struct ADC
    {
        bool adc_debug_en;
        double adc_freq;
        double adc_debug_freq;
        double adc_sys_clk;
    };

    struct DAC
    {
        bool dac_debug_en;
        bool dac_tx_en;
        bool dac_tx_udp_en;
        double dac_freq;
        double dac_sys_clk;
    };

    Base base;
    UDP udp;
    ADC adc;
    DAC dac;
};

#endif // COMMAND_ARGUMENTS_H
