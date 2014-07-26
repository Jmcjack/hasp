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

int read_eventB()
{
	int peakDataB;
	int fd = (int)malloc(sizeof(int));
	spi * spi_ptr = (spi *)malloc(sizeof(spi *));
	spi_ptr->fd = spi_init(PADC_device,PADC_mode,PADC_bpw,PADC_speed);
	fprintf(stderr,"fd = %d.\n",fd);

	int *outputData = (int *)malloc(sizeof(int));

	// generate timestamp
    	struct timespec spec;

    	clock_gettime(CLOCK_REALTIME, &spec);

    	long s  = spec.tv_sec;
    	long us = round(spec.tv_nsec/1000); // Convert nanoseconds to microseconds
	int s_p = (int)s;
	int us_p = (int)us;
	// end timestamp

	uint8_t *tx = (uint8_t *)malloc(2*2*sizeof(uint8_t)); // shouldn't this be just 2?
	uint8_t *rx = (uint8_t *)malloc(2*2*sizeof(uint8_t));

	tx[0] = readCH1;
	tx[1] = 0x00;

	spi_transfer(spi_ptr,tx,rx,4);	// Manually set up the pointer

	outputData[0] = rx[0];
	outputData[1] = rx[1];
	peakDataB = ((outputData[0] & 0x4F) << 8) + outputData[1];

	// Reset ADC channels
	gpio_set_value(EVENTB_RESET_GPIO, 1);
	gpio_set_value(EVENTB_RESET_GPIO, 0);

	// Log data in binary format
	printf("Peak: %d\n", peakDataB);
	FILE * fp;
	fp = open(PEAK_FILENAMEB, "a+");
	fwrite(&s_p, 1, sizeof(&s_p)/2, fp);
    	fwrite(&us_p, 1, sizeof(&us_p)/2, fp);
    	fwrite(&peakDataA, 1, sizeof(&peakDataB), fp);
	fclose(fp);
		
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






	

	
		








