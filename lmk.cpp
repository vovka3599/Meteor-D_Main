#include "lmk.h"

namespace LMK3318 {
    LMK::LMK()
    {

    }

    int LMK::lml_read(uint8_t command, bool print)
    {
        int ret = i2c_smbus_read_byte_data(dev_lmk, command);
        if (print)
            printf("R[%x] = 0x%.2x\n", command, ret);
        return ret;
    }

    void LMK::lmk_write(uint8_t command, uint8_t value)
    {
        i2c_smbus_write_byte_data(dev_lmk, command, value);
    }

    void LMK::lmk_modify(uint8_t command, uint8_t value, uint8_t mask)
    {
        uint8_t ret = lml_read(command, 0);
        ret = (ret | value) & mask;
        i2c_smbus_write_byte_data(dev_lmk, command, ret);
    }

    void LMK::lmk_init()
    {
        unsigned short ret = 0;
        dev_lmk = open(DEV_I2C, O_RDWR);
        if (ioctl(dev_lmk, I2C_SLAVE, DEV_ADRESS) < 0)
        {
            printf("Error: Could not set address\n");
            exit(0);
        }

        lml_read(0, 0);
        lml_read(0, 0);
        lml_read(0, 0);

        int size = sizeof(reg)/sizeof(unsigned short);
        for(int i = 0; i < size; i++)
        {
            lmk_write(reg[i] >> 8, reg[i] & 0x00FF);
            usleep(1000);
        }

        lmk_modify(12, 0x00, 0x7F);
        usleep(100000);
        lmk_modify(12, 0x80, 0xFF);
        usleep(100000);
        lml_read(12, 1);

        lmk_modify(12, 0x00, 0xBF);
        lmk_modify(56, 0x00, 0xFD);

        close(dev_lmk);
    }

    int LMK::i2c_smbus_access(int file, char read_write, uint8_t command, int size, union i2c_smbus_data *data)
    {
        struct i2c_smbus_ioctl_data args;

        args.read_write = read_write;
        args.command = command;
        args.size = size;
        args.data = data;
        return ioctl(file, I2C_SMBUS, &args);
    }

    int LMK::i2c_smbus_read_byte_data(int file, uint8_t command)
    {
        union i2c_smbus_data data;
        i2c_smbus_access(file, I2C_SMBUS_READ, command, I2C_SMBUS_BYTE_DATA, &data); // I2C_SMBUS_BYTE_DATA
        return 0x0FF & data.byte;
    }

    int LMK::i2c_smbus_write_byte_data(int file, uint8_t command, uint8_t value)
    {
        union i2c_smbus_data data;
        data.byte = value;
        return i2c_smbus_access(file, I2C_SMBUS_WRITE, command, I2C_SMBUS_BYTE_DATA, &data);
    }
}
