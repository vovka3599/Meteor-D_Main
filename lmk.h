#ifndef LMK_H
#define LMK_H
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <fstream>
#include <memory.h>
#include <vector>
#include "lmk_reg.h"

#define DEV_I2C       "/dev/i2c-0"
#define DEV_ADRESS      0x50

namespace LMK3318 {
    class LMK
    {
    private:
        int dev_lmk;

    private:
        int i2c_smbus_access(int file, char read_write, uint8_t command, int size, union i2c_smbus_data *data);
        int i2c_smbus_read_byte_data(int file, uint8_t command);
        int i2c_smbus_write_byte_data(int file, uint8_t command, uint8_t value);
        int lml_read(uint8_t command, bool print);
        void lmk_write(uint8_t command, uint8_t value);
        void lmk_modify(uint8_t command, uint8_t value, uint8_t mask);

    public:
        ~LMK();
        LMK();

        /**
         * @brief lmk_init          LMK initialization
         */
        void lmk_init();
        void lmk_stop();
    };
}

#endif // LMK_H
