#ifndef HASP_RTC_H_
#define HASP_RTC_H_
#define BMA180_I2C_BUFFER 0x80

#define I2C_PORT "/dev/i2c-" // change to actual i2c /dev port
#define I2C_Address  0b1101000 // binary address of the RTC

int getTimeStamp();