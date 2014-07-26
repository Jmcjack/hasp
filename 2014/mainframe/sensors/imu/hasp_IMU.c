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
#include "hasp_IMU.h"


int init_IMU(struct imu *imu_ptr)
{
        int fd = (int)malloc(sizeof(int));
        fd = spi_init(IMU_device,IMU_mode,IMU_bpw,IMU_speed);
        fprintf(stderr,"fd = %d.\n",fd);

        imu_ptr->spi_handle = fd;

        return 0;
}


int read_IMU(struct imu *imu_ptr)
{

        uint8_t *tx = (uint8_t *)malloc(12*2*sizeof(uint8_t));
        uint8_t *rx = (uint8_t *)malloc(12*2*sizeof(uint8_t));

        uint8_t *response = (uint8_t *)malloc(12*2*sizeof(uint8_t));
        int *outputData = (int *)malloc(12*sizeof(int));

        int tmp;
        int i;
	int responseLength = 12*2;

        //0x3E00 command begins burst mode output; request all 12 data registers
        tx[0] = 0x3E;
        tx[1] = 0x00;
        spi_transfer(imu_ptr->spi_handle, tx, rx, 2);

        // This pause is necessary to ensure that the sensor has time to respond before the read attempt.
        // The SPI bit rate is 500kHz, with a 15 usec delay in between each 16-bit frame, and a 1 usec
        // delay between the chip select down and clock start/stop. Thus, a 26-byte
        // read/write will take at least 600 usec. This has been measured to be approximately 630 usec.
        usleep(1000);

        // read bytes from buffer into response
        for(i=0;i<12*2;i++) {
                tx[i] = 0x00;
        }

        spi_transfer(imu_ptr->spi_handle, tx, response, 12*2);

        //timestamp the results
        imu_ptr->time = (int)time(NULL);

        /*
        Supply voltage and all inertial sensor readings are 14 bit. Temperature and ADC are
        12 bit. Unused bits are masked after first byte is read. This byte is then shifted up
        8 bits, and the lower byte is read and added to it to obtain the raw value (either 12
        or 14 bit, and either binary or two's complement depending on which register is being
        read). If the number is two's complement, it is converted to decimal.
        */
	
        for(i=2; i<responseLength-4; i+=2)
        {
                tmp = ((response[i] & 0x3F)<<8) + response[i+1];

                if (tmp > 0x1FFF) {
                        outputData[i/2] = -(0x4000-tmp);
                }
                else {
                      	outputData[i/2] = tmp;
                }

        }
	
	int tmp1 = response[0];
	int tmp2 = response[1];
	outputData[0] = ((tmp1 & 0x3F) << 8 ) + tmp2;
	tmp1 = response[20];
	tmp2 = response[22];
	tmp1 = ((tmp1 & 0x0F) << 8) +tmp2;
	if(tmp1 > 0x07FF)
	{
		tmp1 = -(0x1000 - tmp1);
	}	
	outputData[10] = tmp1;
	tmp1 = response[22];
	tmp2 = response[23];
	outputData[11] = ((tmp1 & 0x0F) << 8) + tmp2;
		
        // set status flag if supply voltage is within 4.75 to 5.25
        if(outputData[0]*0.002418 < 5.25 && outputData[0]*0.002418 > 4.75)
	{
                fprintf(stderr,"data_valid\n");
                imu_ptr->supply = outputData[0]*0.002418;
                imu_ptr->gyroX  = (int)outputData[1]*0.05;
                imu_ptr->gyroY  = (int)outputData[2]*0.05;
                imu_ptr->gyroZ  = (int)outputData[3]*0.05;
                imu_ptr->accelX = (int)outputData[4]*0.00333*g;
                imu_ptr->accelY = (int)outputData[5]*0.00333*g;
                imu_ptr->accelZ = (int)outputData[6]*0.00333*g;
                imu_ptr->magX = (int)outputData[7]*0.0005;
                imu_ptr->magY = (int)outputData[8]*0.0005;
                imu_ptr->magZ = (int)outputData[9]*0.0005;
                imu_ptr->temp = (int)outputData[10]*0.14;
                imu_ptr->ADC = outputData[11]*0.000806;
        }
        else
	{
                fprintf(stderr,"data_invalid\n");
        }

        free(tx);
        free(rx);
        free(response);
        free(outputData);

        return 0;

}

int close_IMU(struct imu *imu_ptr)
{
        close(imu_ptr->spi_ptr->fd);
        return 0;
}


