#ifndef IMU_ISENSOR_H_
#define IMU_ISENSOR_H_

static const char* IMU_port = "/dev/spidev1.0";		///< SPI port for IMU
static uint32_t IMU_speed = 1000000; 				///< Target Frequency
static uint8_t IMU_mode = 3;						///< SPI Mode
static uint8_t IMU_bits = 8;						///< Bits per word
static uint16_t IMU_delay = 100;					///< [us] Delay between bit transfers 

//#define BUGGER		// enable SPI Debugging

#endif /* IMU_ISENSOR_H_ */
