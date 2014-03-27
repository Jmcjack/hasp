/*! \file gps_crescent.c
 *	\brief Hemisphere Crescent OEM GPS receiver source code
 *
 *	\details This file implements the init_gps() and read_gps() functions for the Hemisphere Crescent OEM GPS receiver.
 *	\ingroup gps_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: gps_crescent.c 760 2012-01-12 16:00:45Z murch $
 */

#include <termios.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include <pthread.h>
#include <sched.h>
#include <cyg/posix/pthread.h>
#include <cyg/kernel/kapi.h>

#include "../../globaldefs.h"
#include "gps_crescent.h"
#include "gps_interface.h"
#include "../../utils/serial_mpc5200.h"
#include "../../utils/misc.h"
#define CRC32_POLYNOMIAL   0xEDB88320L

static unsigned char localBuffer[512];
static unsigned char header_length = 28;

// Initialize GPS and configure it to send the desired messages
void init_gps(struct gps *gpsData_ptr){
	//	gpsData_ptr->port = open_serial(gpsData_ptr->portName, gpsData_ptr->baudRate );
	gpsData_ptr->port = open_serial(SERIAL_PORT2, B115200);
}

// How to handle static variables with multiple sensors?  objects? add to gpspacket?
int read_gps(struct gps *gpsData_ptr)
{
	cyg_io_handle_t port_handle;
	cyg_serial_buf_info_t buff_info;
	unsigned int len = sizeof (buff_info);

	// get serial port handle
	cyg_io_lookup( gpsData_ptr->portName, &port_handle );
	cyg_io_get_config (port_handle, CYG_IO_GET_CONFIG_SERIAL_BUFFER_INFO,&buff_info, &len);

	/*	if (gpsData_ptr->localBuffer == NULL)
	{
		gpsData_ptr->localBuffer = (unsigned char*) malloc (1024 * sizeof (unsigned char));
	}
	 */	

	unsigned int bytesInLocalBuffer;
	unsigned int state = 0;
	unsigned int counter = 0;
	unsigned int CRC_computed;
	unsigned int CRC_read;
	int i=0;
	int pCount=0;
	unsigned char CRC_readstr[4];

	bytesInLocalBuffer = read(gpsData_ptr->port,&localBuffer[0],512);
//	printf("\nbytes read = %d \n",bytesInLocalBuffer);

	//---------------------------------------------------------
	// Exit read_gps function in case entire message isn't read
	if(bytesInLocalBuffer<144)
	{return 0;}
	//---------------------------------------------------------

	//for(counter=0;counter<bytesInLocalBuffer;counter++){	
	//	printf("%0x ", localBuffer[counter]);		
	//}

	while(counter<bytesInLocalBuffer){
		switch(state){
		case 0:
			if(localBuffer[counter]==0xAA){
//				fprintf(stderr,"\nRead AA\n");
				state++;
				counter++;
			}else{
				state = 0;
				counter++;
			}
			break;
		case 1:
			if(localBuffer[counter]==0x44){
//				fprintf(stderr,"Read 44\n");
				state++;
				counter++;
			}else{
				state = 0;
				counter++;
			}
			break;
		case 2:
			if(localBuffer[counter]==0x12){
//				fprintf(stderr,"Read 12\n");
				state++;
				//counter -2 is the start of your packet
			}else{
				state=0;
				counter++;
			}
			break;
		case 3:
			//fprintf(stderr,"counter = %d\n", counter);
			// Calculate CRC
			endian_swap(localBuffer, counter-2+140, 4);
			//fprintf(stderr,"%0X %0X %0X",*(localBuffer+counter-2),*(localBuffer+counter-2+1),*(localBuffer+counter-2+2));

			CRC_computed = CalculateBlockCRC32(140,localBuffer+counter-2);

			//fprintf(stderr,"CRC\n");
			for (i = 0; i<4;i++) {
				CRC_readstr[i] = localBuffer[counter-2+140+i];
				//	fprintf(stderr,"%0X ",localBuffer[counter-2+140+i]);
			}
			//fprintf(stderr,"\n");

			CRC_read = *( (unsigned int*) CRC_readstr);
			//fprintf(stderr,"CRC Calculated = %d, CRC read = %d\n",CRC_computed,CRC_read);

			if (CRC_computed != CRC_read) {
				state = 0;
				counter = bytesInLocalBuffer;
				fprintf(stderr,">>CRC Err\n");
			}
			else {
				pCount = counter - 2;
				//fprintf(stderr,"\nSize of float = %d, size of double = %d\n",sizeof(float),sizeof(double));

				endian_swap(localBuffer,pCount+14,2);
				endian_swap(localBuffer,pCount+16,4);

				gpsData_ptr->GPS_week = ((double)*((unsigned short *)(&localBuffer[pCount+14])));
//				printf("\nGPS_week: %d",gpsData_ptr->GPS_week);

				gpsData_ptr->GPS_TOW = ((double)*((long *)(&localBuffer[pCount+16])));
//				printf("\nGPS_TOW: %f\n",gpsData_ptr->GPS_TOW);

				endian_swap(localBuffer,pCount+28+8,8);
				endian_swap(localBuffer,pCount+28+16,8);
				endian_swap(localBuffer,pCount+28+24,8);
				endian_swap(localBuffer,pCount+28+32,4);
				endian_swap(localBuffer,pCount+28+36,4);
				endian_swap(localBuffer,pCount+28+40,4);
				endian_swap(localBuffer,pCount+28+52,8);
				endian_swap(localBuffer,pCount+28+60,8);
				endian_swap(localBuffer,pCount+28+68,8);
				endian_swap(localBuffer,pCount+28+76,4);
				endian_swap(localBuffer,pCount+28+80,4);
				endian_swap(localBuffer,pCount+28+84,4);
				gpsData_ptr->Xe = *((double *)(&localBuffer[pCount+header_length+8]));
				gpsData_ptr->Ye = *((double *)(&localBuffer[pCount+header_length+16])); 
				gpsData_ptr->Ze = *((double *)(&localBuffer[pCount+header_length+24]));
				gpsData_ptr->Px = (double)(*((float *)(&localBuffer[pCount+header_length+32])));
				gpsData_ptr->Py = (double)(*((float *)(&localBuffer[pCount+header_length+36])));
				gpsData_ptr->Pz = (double)(*((float *)(&localBuffer[pCount+header_length+40])));
				gpsData_ptr->Ue = *((double *)(&localBuffer[pCount+header_length+52]));
				gpsData_ptr->Ve = *((double *)(&localBuffer[pCount+header_length+60]));
				gpsData_ptr->We = *((double *)(&localBuffer[pCount+header_length+68]));
				gpsData_ptr->Pu = (double)(*((float *)(&localBuffer[pCount+header_length+76])));
				gpsData_ptr->Pv = (double)(*((float *)(&localBuffer[pCount+header_length+80])));
				gpsData_ptr->Pw = (double)(*((float *)(&localBuffer[pCount+header_length+84])));
			}
			state = 0;
			counter = bytesInLocalBuffer;
			break;
		}
	}
	return 0;
}


// --------------------------------------------------------------------------
//Calculate a CRC value to be used by CRC calculation functions. 
//-------------------------------------------------------------------------- 
unsigned int CRC32Value(int i)
{
	int j;
	unsigned int ulCRC;
	ulCRC = i;
	for ( j = 8 ; j > 0; j-- )
	{
		if ( ulCRC & 1 )
			ulCRC = ( ulCRC >> 1 ) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

//--------------------------------------------------------------------------
//Calculates the CRC-32 of a block of data all at once
//--------------------------------------------------------------------------
unsigned int CalculateBlockCRC32(
		unsigned int ulCount,     //Number of bytes in the data block
		unsigned char *ucBuffer)  //Data block
{
	unsigned int ulTemp1;
	unsigned int ulTemp2;
	unsigned int ulCRC = 0;
	while ( ulCount-- != 0 )
	{
		ulTemp1 = ( ulCRC >> 8 ) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value( ((int) ulCRC ^ *ucBuffer++ ) & 0xff );
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return( ulCRC );
}


