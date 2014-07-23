#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include "../../globaldefs.h"
#include "../../utilities/spi/spi.h"
#include "peak_ADC.h"
// #include "read_RTC.h" 

int read_event()
{
	int peakDataA;
	int peakDataB;
	int fd = (int)malloc(sizeof(int));
	spi * spi_ptr = (spi *)malloc(sizeof(spi *));
	spi_ptr->fd = spi_init(PADC_device,PADC_mode,PADC_bpw,PADC_speed);
	fprintf(stderr,"fd = %d.\n",fd);

	int *outputData = (int *)malloc(2*sizeof(int));

	//generate timestamp from RTC?
	//type timestamp = read_RTC()"
	uint8_t *tx = (uint8_t *)malloc(4*2*sizeof(uint8_t));
	uint8_t *rx = (uint8_t *)malloc(4*2*sizeof(uint8_t));

	tx[0] = readCH0;
	tx[1] = 0x00;
	tx[2] = readCH1;
	tx[3] = 0x00;

	spi_transfer(spi_ptr,tx,rx,4);	// Manually set up the pointer

	outputData[0] = rx[0];
	outputData[1] = rx[1];
	peakDataA = ((outputData[0] & 0x4F) << 8) + outputData[1];
	outputData[0] = rx[2];
	outputData[1] = rx[3];
	peakDataB = ((outputData[0] & 0x4F) << 8) + outputData[1];

	// Reset both Ch (using WR_GPIO function?)

	// Logdata
		
	free(tx);
	free(rx);
	free(outputData);
	close(spi_ptr->fd);

	return 0;
}

int main() 
{
	read_event();
	return 0;
}

