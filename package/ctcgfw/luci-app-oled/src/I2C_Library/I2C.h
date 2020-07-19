/*
 * MIT License

Copyright (c) 2017 DeeplyEmbedded

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 * I2C.h
 *
 *  Created on  : Sep 4, 2017
 *  Author      : Vinay Divakar
 *  Website     : www.deeplyembedded.org
 */ 

#ifndef I2C_H_
#define I2C_H_

#include<stdint.h>

/* No. of bytes per transaction */
#define I2C_ONE_BYTE                     1
#define I2C_TWO_BYTES                    2
#define I2C_THREE_BYTES                  3

/*Definitions specific to i2c-x */
#define I2C_DEV0_PATH                     "/dev/i2c-0"
#define I2C_DEV1_PATH                     "/dev/i2c-1"
#define I2C_DEV2_PATH                     "/dev/i2c-2"

/*I2C device configuration structure*/
typedef struct{
	char* i2c_dev_path;
	int fd_i2c;
	unsigned char i2c_slave_addr;
}I2C_DeviceT, *I2C_DevicePtr;

/* Exposed Generic I2C Functions */
extern int Open_device(char *i2c_dev_path, int *fd);
extern int Close_device(int fd);
extern int Set_slave_addr(int fd, unsigned char slave_addr);
extern int i2c_write(int fd, unsigned char data);
extern int i2c_read(int fd, unsigned char *read_data);
extern int i2c_read_register(int fd, unsigned char read_addr, unsigned char *read_data);
extern int i2c_read_registers(int fd, int num, unsigned char starting_addr,
		unsigned char *buff_Ptr);
extern void config_i2c_struct(char *i2c_dev_path, unsigned char slave_addr, I2C_DevicePtr i2c_dev);
extern int i2c_multiple_writes(int fd, int num, unsigned char *Ptr_buff);
extern int i2c_write_register(int fd, unsigned char reg_addr_or_cntrl, unsigned char val);

/* Exposed I2C-x Specific Functions */
extern int init_i2c_dev(const char* i2c_path, unsigned char slave_address);

#endif /* I2C_H_ */
