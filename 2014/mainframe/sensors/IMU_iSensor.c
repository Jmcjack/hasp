// This driver is being adapted from the UMN's UAV Laboratory for the iSensor IMU over an SPI protocol for the Beaglebone Black.

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

//#include "../../globaldefs.h"
//#include "../../utils/misc.h"
#include "IMU_iSensor.h"
#include "imu_interface.h"


// Initialize SPI communication
int init_imu() {	

}

int read_imu(struct imu *imuData_ptr) {

	//timestamp the results
	imuData_ptr->time = get_Time();

	int i;
	byte response[26]={0};
	double outputData[13];
	uint32_t commandLength, responseLength;
	uint16_t tmp;

	uint16_t command[26]={0};

	// burst mode output; request all 13 data registers
	command[0] = 0x3E00;
	commandLength = responseLength = 13*2; //bytes to read/write
	
	// Send command
	cyg_io_write( hSPI, command, &commandLength); 
	
	/* This pause is necessary to ensure that the sensor has time to respond before the read attempt.
	 * The SPI bit rate is 500kHz, with a 15 usec delay in between each 16-bit frame, and a 1 usec
	 * delay between the chip select down and clock start/stop. Thus, a 26-byte
	 * read/write will take at least 600 usec. This has been measured to be approximately 630 usec.
	 * Note 1 tick for cyg_thread_delay is 100 usec.
	 */
	cyg_thread_delay(10); 
	
	// read bytes from buffer into response
	cyg_io_read( hSPI, response, &responseLength );	
	
	#ifdef verbose
		fprintf( stderr,"Read %d SPI bytes\n",responseLength);
		for(i=0; i<responseLength;i++){
			printf("%2.2x ",response[i]) ;
		}
		printf("\n");
	#endif
	
	

	 // All inertial sensor outputs are in 14-bit, twos complement format
	 // Combine data bytes; each regsister covers two bytes, but uses only 14 bits.
	for ( i = 0; i < responseLength; i += 2 ) {
		 tmp = (response[i] & 0x3F) * 256 + response[i+1];
		
		if ( tmp > 0x1FFF ) {
			outputData[i/2] = (double)(-(0x4000 - tmp));
		}
		else
		outputData[i/2] = (double)tmp;		
	}
	
	// set status flag if supply voltage is within 4.75 to 5.25
	if ((outputData[1]*0.002418 > 4.25) && (outputData[1]*0.002418 < 5.25)){
	
		imuData_ptr->err_type = data_valid;
		// update imupacket
		// *IMP* IMU axis alignment is different: Z=-Z_body, Y=-Y_body
		imuData_ptr->Vs = outputData[1]*0.002418;	//unit: Volt
		imuData_ptr->p  = D2R * outputData[2]*0.05;		//unit: rad/s
		imuData_ptr->q  = -D2R * outputData[3]*0.05;
		imuData_ptr->r  = -D2R * outputData[4]*0.05;
		imuData_ptr->ax = -g * outputData[5]*0.00333;		//unit: m/s^2
		imuData_ptr->ay = g * outputData[6]*0.00333;
		imuData_ptr->az = g * outputData[7]*0.00333;
		imuData_ptr->hx = outputData[8]*0.0005;			//unit: Gauss
		imuData_ptr->hy = outputData[9]*0.0005;
		imuData_ptr->hz = outputData[10]*0.0005;
		imuData_ptr->T  = 25.0 + outputData[11]*0.14;
		imuData_ptr->adc = outputData[12]*0.000806;	//unit: Volt
	}
	else{
		imuData_ptr->err_type = got_invalid;
	}
	
	// output real units
#ifdef verbose
		fprintf( stderr, "\n" );
		fprintf( stderr, "power = %.2f V\n", imuData_ptr->Vs );
		fprintf( stderr, "X gyro = %.2f deg/s\n", imuData_ptr->p );
		fprintf( stderr, "Y gyro = %.2f deg/s\n", imuData_ptr->q );
		fprintf( stderr, "Z gyro = %.2f deg/s\n", imuData_ptr->r );
		fprintf( stderr, "X accel = %.2f G\n", imuData_ptr->ax );
		fprintf( stderr, "Y accel = %.2f G\n", imuData_ptr->ay );
		fprintf( stderr, "Z accel = %.2f G\n", imuData_ptr->az );
		fprintf( stderr, "X mag = %.2f mgauss\n", imuData_ptr->hx );
		fprintf( stderr, "Y mag = %.2f mgauss\n", imuData_ptr->hy );
		fprintf( stderr, "Z mag = %.2f mgauss\n", imuData_ptr->hz );
		fprintf( stderr, "Temp = %.1f C\n",	imuData_ptr->T );
		fprintf( stderr, "ADC = %.2f V\n",	imuData_ptr->adc );
#endif

	return 0;
}
