#ifndef HASP_RTC_H_
#define HASP_RTC_H_
#define BMA180_I2C_BUFFER 0x80

#define I2C_PORT "/dev/i2c-" // change to actual i2c port
#define I2C_SLAVE // something, but what?
#define I2C_Address // something, but what?

int getTimeStamp();