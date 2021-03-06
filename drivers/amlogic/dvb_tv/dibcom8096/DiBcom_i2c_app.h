#ifndef __DIBCOM_I2C_HHHHHHHHHH
#define __DIBCOM_I2C_HHHHHHHHHH

#include "dibcomfrontend.h"
#include <linux/i2c.h>

//dibcom write
unsigned char DiBcom_i2c_write(int addr, unsigned char *i2c_tx_buf, int txcnt);

//dibcom before read , should write the register address first (write without stop)
unsigned char DiBcom_i2c_read(int addr, unsigned char *i2c_tx_buf, int txcnt, unsigned char  *i2c_rx_buf, int rxcnt);

int getChipId(void);

int test_dibcom();
#endif
