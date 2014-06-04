#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "hasp_spi.h"
#include "../debugging_utils/debugging_utils.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

int hasp_spi_init(struct spi spi_ptr)
{
	int status = 0;			// Used to check status of function returns
	int spi_handle;			// Handle to the SPI Port

	spi_handle = open(spi_ptr->device, O_RDWR);
	if (spi_handle < 0)
	{
		#ifdef BUGGER
			printf("SPI Cannot open device!\n");
		#endif
		
	}

	// SPI Mode
	status = ioctl(fd, SPI_IOC_WR_MODE, &spi_ptr->mode);
	if (status == -1)
	{
		#ifdef BUGGER
			printf("SPI Mode cannot be set!\n");
		#endif
	}
	status = ioctl(fd, SPI_IOC_RD_MODE, &spi_ptr->mode);
	if (status == -1)
	{
		#ifdef BUGGER
			printf("SP Mode cannot be retrieved!\n");
		#endif
	}

	// Bits per Word
	status = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi_ptr->bits);
	if (status == -1)
	{
		#ifdef BUGGER
			printf("SPI Bits per Word cannot be set!\n");
		#endif
	}

	status = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spi_ptr->bits);
	if (status == -1)
	{
		#ifdef BUGGER
			printf("SPI Bits per Word cannot be retrieved!\n");
		#endif
	}

	// Max Speed
	status = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_ptr->speed);
	if (status == -1)
	{
		#ifdef BUGGER
			printf("SPI Max Speed cannot be set!\n");
		#endif
	}

	status = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_ptr->speed);
	if (status == -1)
	{
		#ifdef BUGGER
			printf("SPI Max Speed cannot be gotten!\n");
		#endif	
	}

	#ifdef BUGGER
		printf("SPI Port Open Success!\n");
		printf("Device: ");
		printArray(spi_ptr->device);
		printf("Mode: %d\n", spi_ptr->mode);
		printf("Bits: %d\n", spi_ptr->bits);
		printf("Speed [Hz]: %d Hz (%d MHz)\n", spi_ptr->speed, spi_ptr->speed/1000000);
	#endif

	return spi_handle;
}

int hasp_spi_close(int spi_handle)
{
	int status = 0;	
	// status_value = close(spi_handle);
	// if(status < 0)
	//  Exit ungracefully
	return status;
}

int hasp_spi_transfer(struct spi spi_ptr, char* data, int length)
{
	struct spi_ioc_transfer spi[length];
	int i=0;
	int status = 0;
	int spi_handle = spi_ptr->hande;
	// Determine which SPI port handle to use
	// Once transfer per byte
	for (i = 0 ; i < length ; i++)
	{
		spi[i].tx_buf        = (unsigned long)(data + i); // transmit from "data"
		spi[i].rx_buf        = (unsigned long)(data + i); // receive into "data"
		spi[i].len           = sizeof(*(data + i));
		spi[i].delay_usecs   = spi_ptr->delay;
		spi[i].speed_hz      = spi_ptr->speed;
		spi[i].bits_per_word = spi_ptr->bits;
		spi[i].cs_change = 0;
	}
	status = ioctl(spi_handle, SPI_IOC_MESSAGE(length), &spi);
	
	if(status < 0)
	{
		// Exit ungracefully
	}
	
	spi_ptr->rx_data = spi.rx_buff;
	
	return status;
}