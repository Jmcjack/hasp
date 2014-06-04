#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static uint8_t mode;
static uint8_t bits = 8;
static uint16_t delay;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

int hasp_spi_init(const char* device, uint32_t speed)
{
	int status = 0;	// Used to check status of function returns
	int spi_handle;			// Handle to the SPI Port

	spi_handle = open(device, O_RDWR);
	if (spi_handle < 0)
		printf("SPI Cannot open device!\n");

	// SPI Mode
	status = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (status == -1)
		printf("SPI Mode cannot be set!\n");

	status = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (status == -1)
		printf("SP Mode cannot be retrieved!\n");

	// Bits per Word
	status = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (status == -1)
		printf("SPI Bits per Word cannot be set!\n");

	status = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (status == -1)
		printf("SPI Bits per Word cannot be retrieved!\n");

	// Max Speed
	status = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (status == -1)
		printf("SPI Max Speed cannot be set!\n");

	status = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (status == -1)
		printf("SPI Max Speed cannot be gotten!\n");

	#ifdef BUGGER
		printf("SPI Port Open Success!\n");
		printf("Device: ");
		printArray(device);
		printf("Mode: %d\n", mode);
		printf("Bits: %d\n", bits);
		printf("Speed [Hz]: %d Hz (%d MHz)\n", speed, speed/1000000);
	#endif

	return 1;
}

int hasp_spi_close(const char* device)
{
	int status = 0;
	int spi_handle;
	
	// status_value = close(spi_handle);
	// if(status < 0)
	//  Exit ungracefully
	return status;
}

int hasp_spi_transfer(const char* device, unsigned char* data, int length)
{
	struct spi_ioc_transfer spi[length];
	int i=0;
	int status = 0;
	int spi_handle;
	// Determine which SPI port handle to use
	// Once transfer per byte
	for (i = 0 ; i < length ; i++)
	{
		spi[i].tx_buf        = (unsigned long)(data + i); // transmit from "data"
		spi[i].rx_buf        = (unsigned long)(data + i) ; // receive into "data"
		spi[i].len           = sizeof(*(data + i)) ;
		spi[i].delay_usecs   = 0 ;
		spi[i].speed_hz      = spi_speed ;
		spi[i].bits_per_word = spi_bitsPerWord ;
		spi[i].cs_change = 0;
	}
	status = ioctl(spi_handle, SPI_IOC_MESSAGE(length), &spi);
	
	if(status < 0)
	{
		// Exit ungracefully
	}
	
	return status;
}