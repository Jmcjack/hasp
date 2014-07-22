#ifndef _HASP_SPI_DEVICES_H_
#define _HASP_SPI_DEVICES_H_

#include <stdint.h>
#include "../../globaldefs.h"
//#include "spi.h"

// IMU
static const char *IMU_device = "/dev/spidev2.0";
static uint8_t IMU_mode = 3;
static uint8_t IMU_bpw = 8;
static uint32_t IMU_speed = 1000000;
static uint16_t IMU_delay = 100;


// PEAK ADC
// The peak detection SPI data transfer is done in the kernel

//static const char *PADC_device = "/dev/spidev1.0";
//static uint8_t PADC_mode = 0;
//static uint8_t PADC_bpw = 8;
//static uint32_t PADC_speed = 3200000;
//static uint16_t PADC_delay=100;

int init_IMU(struct imu *imu_ptr);
int read_IMU(struct imu *imu_ptr);
int close_IMU(struct imu *imu_ptr);


#endif





