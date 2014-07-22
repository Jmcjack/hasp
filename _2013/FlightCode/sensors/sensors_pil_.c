/*! \file sensors_pil.c
 *	\brief Data acquisition source code for PIL simulation
 *
 *	\details This file implements the init_daq() and daq() functions for the PIL simulation. Acquires data from PIL simulation over serial
 * 	port of MPC5200 and emulates IMU, GPS, and Air Data sensors.
 * 	\ingroup daq_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: sensors_pil.c 761 2012-01-19 17:23:49Z murch $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <cyg/posix/pthread.h>
#include <cyg/kernel/kapi.h>
#include <cyg/cpuload/cpuload.h>
#include <cyg/gpt/mpc5xxx_gpt.h>
#include <cyg/io/mpc5xxx_gpio.h>
#include <cyg/io/i2c_mpc5xxx.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>

#include "../globaldefs.h"
#include "../utils/serial_mpc5200.h"
#include "../utils/misc.h"
#include "../utils/scheduling.h"
#include "../extern_vars.h"
#include "AirData/airdata_interface.h"
#include "GPS/gps_interface.h"
#include "IMU/imu_interface.h"
#include "ADC/adc_interface.h"
#include "GPIO/gpio_interface.h"
#include AIRCRAFT_UP1DIR

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//PIL packet length, in bytes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define PACKET_LENGTH		156

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//prototype definition
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void read_pil_serial();
void reset_localBuffer();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Internal Variables
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static unsigned char* localBuffer =  NULL;
static int bytesInLocalBuffer = 0, readState = 0;
static cyg_io_handle_t pilPort_handle;
static cyg_serial_buf_info_t buff_info;
static double pil_data[21];

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GPIO Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void init_gpio(){}

void read_gpio(struct control *controlData_ptr){
	if(pil_data[1] == 1){
		controlData_ptr->mode = 0; // data dump
	}
	else{
		controlData_ptr->mode = pil_data[0]+1; // manual or auto mode
	}
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IMU Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int init_imu(){
	// get serial port handle
	cyg_io_lookup( PIL_PORT, &pilPort_handle );	
	pilPort = open_serial(PIL_PORT,PIL_BAUDRATE);
	return 1;
}

int read_imu(struct imu *imuData_ptr)
{
	read_pil_serial();
	
	imuData_ptr->time = get_Time();
	imuData_ptr->err_type = data_valid;
	
	/* angular rate in rad/s */
	imuData_ptr->p = pil_data[2];  
	imuData_ptr->q = pil_data[3];  
	imuData_ptr->r = pil_data[4];  
	
	/* acceleration in m/s^2 */	
	imuData_ptr->ax = pil_data[5];  
	imuData_ptr->ay = pil_data[6];  
	imuData_ptr->az = pil_data[7];  
	
	/* magnetic field in Gauss */
	imuData_ptr->hx = pil_data[8];  
	imuData_ptr->hy = pil_data[9];  
	imuData_ptr->hz = pil_data[10]; 	
	return 1;
}
	
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// GPS Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void init_gps(struct gps *gpsData_ptr){}

int read_gps(struct gps *gpsData_ptr)
{
	static int count = 0;

	if(++count > BASE_HZ){
		gpsData_ptr->newData = 1; // set newData flag at 1Hz				
		count = 0;
		/* gps position */
		gpsData_ptr->lat = pil_data[11];
		gpsData_ptr->lon = pil_data[12];
		gpsData_ptr->alt = pil_data[13];

		/* gps velocity in m/s */
		gpsData_ptr->vn = pil_data[14];
		gpsData_ptr->ve = pil_data[15];
		gpsData_ptr->vd = pil_data[16];

		// Set err_type fields
		gpsData_ptr->err_type = data_valid;
		gpsData_ptr->navValid = 0;	
		return 1;
	}
	else
		return -1;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Air Data Functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void init_airdata(){
}

int read_airdata(struct airdata *adData_ptr)
{	
	adData_ptr->bias[0] = 0.0; // biases for air data
	adData_ptr->bias[1] = 0.0; // biases for air data
	adData_ptr->aoa = pil_data[17]; // aoa
	adData_ptr->aos = pil_data[18]; // aos
	adData_ptr->Pd = pil_data[19]; // Pd
	adData_ptr->Ps = pil_data[20]; // Ps
	return 0;
}
	
	

void read_pil_serial()
{
unsigned int	len, bytesInBuffer;//my_chksum,rcv_chksum;
unsigned int bytesReadThisCall =0;
unsigned short bytesToRead = 0, bytesRead = 0;

// Initialization of persistent local buffer
if (localBuffer == NULL)
{
	localBuffer = (unsigned char*) malloc (1024 * sizeof (unsigned char));
}
	

cyg_io_get_config (pilPort_handle, CYG_IO_GET_CONFIG_SERIAL_BUFFER_INFO,&buff_info, &len);	
	
bytesInBuffer = buff_info.rx_count;

//printf("\nbytesInbuffer = %d",bytesInBuffer);

// Return if no bytes in buffer
if (bytesInBuffer < 1)
	return;
	
// Keep reading until we've processed all of the bytes in the buffer
	while (bytesReadThisCall < bytesInBuffer){
	
		switch (readState){
			case 0: //Look for packet header bytes
				// Read in up to 2 bytes to the first open location in the local buffer
				bytesRead = read(pilPort,&localBuffer[bytesInLocalBuffer],2-bytesInLocalBuffer);
				bytesReadThisCall += bytesRead; // keep track of bytes read during this call
					
				if (localBuffer[0] == 0xAA){ // Check for first header byte
					bytesInLocalBuffer = 1;
					if (localBuffer[1] == 0xAA){ // Check for second header byte
						bytesInLocalBuffer = 2;
						readState++; // header complete, move to next stage
					}
				}
				break;

			case 1: //Read payload, checksum, and stop bytes
								
				// Find how many bytes need to be read for the total message
				bytesToRead = PACKET_LENGTH - bytesInLocalBuffer;
				
				// Read in the remainder of the message to the local buffer, starting at the first empty location
				bytesRead = read (pilPort, &localBuffer[bytesInLocalBuffer], bytesToRead);
				bytesInLocalBuffer += bytesRead; // keep track of bytes in local buffer
				bytesReadThisCall += bytesRead; // keep track of bytes read during this call
				
				if (bytesRead == bytesToRead)
				{
					readState = 0; // reset readState counter as all bytes for this packet have been read
					
/* 					// Compute checksum & compare to received checksum 
					my_chksum = do_chksum(localBuffer, 2, PACKET_LENGTH-2); 
					memcpy(&rcv_chksum, &localBuffer[PACKET_LENGTH-1],2); 
						 
					if(my_chksum == rcv_chksum){                                                                             
						// Extract data 
						                                          
						//trigger AHRS 
						pthread_cond_signal(&trigger_ahrs); 
						} 
					else{ 
						imuData_ptr->err_type = checksum_err; 
					}  */
						
						// copy bytes in localBuffer to double array "pil_data"
						pil_data[0] = (double)localBuffer[2]; // control mode is one byte
						pil_data[1] = (double)localBuffer[3]; // data dump is one byte
						memcpy(&pil_data[2], &localBuffer[4], 152); // remainder of data are 8byte doubles

						reset_localBuffer();
						
				}
				break;	
				
				default:
					reset_localBuffer();
					readState = 0;
					
				break;
		
		}
	
	} // end bytesReadThisCall while loop
			
}



void reset_localBuffer(){
	int i;
	// Clear the first two bytes of the local buffer
	for(i=0;i<2;i++)
		localBuffer[i] = '\0';
	
	bytesInLocalBuffer = 0;
	
	readState = 0; // reset readState counter as all bytes for this packet have been read
}
