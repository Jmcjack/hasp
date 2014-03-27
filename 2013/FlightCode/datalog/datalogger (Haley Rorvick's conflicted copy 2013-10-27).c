/*! \file datalogger.c
 *	\brief Data logging source code
 *
 *	\details Logs data in MATLAB format onboard the MPC5200B in RAM memory.
 *	\ingroup datalog_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: datalogger.c 761 2012-01-19 17:23:49Z murch $
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
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>

#include <dirent.h>
#include <sys/stat.h>
#include <cyg/fileio/fileio.h>
#include <pkgconf/io_fileio.h>
#include <sys/socket.h>
#include <network.h>
#include <tftp_support.h>		// TFTP support
#include <netdb.h>
#include <arpa/inet.h>
#include <pkgconf/fs_ram.h>

#include "../globaldefs.h"
#include "../extern_vars.h"
#include "../utils/misc.h"
#include "datalog_interface.h"






#include "../utils/serial_mpc5200.h"
#include AIRCRAFT_UP1DIR




#define TELE_PACKET_SIZE  51
extern char statusMsg[103];	
#define RAMFILE_NAME	  	"/flightdata.ram"

// Global datalog structure. Defined in main.c
extern struct datalog dataLog;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Internal Function Prototypes
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int createfile( char *name );
int tftpTransfer(char *outfile, char *txbuf, int len);
char *tftp_error(int err);
int filetransferToGCS();
int umountFilesystem();
int writeMATLAB ();


#define SHOW_RESULT( _fn, _res ) \
		diag_printf("\n<FAIL>: " #_fn "() returned %ld %s\n",  \
				(unsigned long)_res, _res<0?strerror(errno):"");

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Internal Variables
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Index of the current datapoint in the arrays
static int32_t currentDataPoint, currentXrayPoint;

// Data arrays
static double* doubleData[100];
static float* floatData[100];
// static float xrayData[100];
static int32_t* intData[100];
static uint16_t* shortData[100];

// Pointer to active buffer
static float active1[100];		
static float active2[100];		
static float *p1;  
static float *p2;
static float *temp1;
int counter1;

// duplicate active arrays (active3 and active 4) and pointers (p3 and p4) for peak
static float active3[100];		
static float active4[100];		
static float *p3;  
static float *p4;
static float *temp2;
int counter2;

//Initializes the arrays needed for logging, Returns 0 on success, non-zero otherwise
int init_datalogger()
{
	int i;
	printf("Begin Logging:\n\r");
	printf("Data,");
	for (i = 0; i < dataLog.numDoubleVars; i++)
		{
		printf(dataLog.saveAsDoubleNames[i]);
		printf(",");
		}
	for (i = 0; i < dataLog.numFloatVars; i++)
		{
		printf(dataLog.saveAsFloatNames[i]);
		printf(",");
		}
	for (i = 0; i < dataLog.numIntVars; i++)
		{
		printf(dataLog.saveAsIntNames[i]);
		printf(",");
		}
	printf("Done\n\r");
	 
	currentDataPoint = 0;
	currentXrayPoint = 0;
	
	p1=active1;
	p2=active2;
	p3=active3;
	p4=active4;
	
	if (sizeof(double) != 8){
		fprintf (stderr, "\nFATAL: double is wrong size: %d", sizeof(double));
		return -1;
	}
	
	if (sizeof(float) != 4){
		fprintf (stderr, "\nFATAL: float is wrong size: %d", sizeof(float));
		return -1;
	}	
	
	for (i = 0; i < dataLog.numDoubleVars; i++){
		doubleData[i] = malloc (sizeof(double) * dataLog.logArraySize);
		if (doubleData[i] == NULL){
			fprintf(stderr, "\nFATAL: Couldn't allocate space for double logging arrays! Memory NOT freed");
			return -1;
		}
	}
	
	for (i = 0; i < dataLog.numFloatVars; i++){
		floatData[i] = malloc (sizeof(float) * dataLog.logArraySize);
		if (floatData[i] == NULL){
			fprintf(stderr, "\nFATAL: Couldn't allocate space for float logging arrays! Memory NOT freed");
			return -1;
		}
	}
	
//	for (i = 0; i < dataLog.numXrayVars; i++){
//		xrayData[i] = malloc (sizeof(float) * dataLog.logArraySize);
//		if (xrayData[i] == NULL){
//			fprintf(stderr, "\nFATAL: Couldn't allocate space for float logging arrays! Memory NOT freed");
//			return -1;
//		}
//	}
	
	for (i = 0; i < dataLog.numIntVars; i++){
		intData[i] = malloc (sizeof(int32_t) * dataLog.logArraySize);
		if (intData[i] == NULL){
			fprintf(stderr, "\nFATAL: Couldn't allocate space for int logging arrays! Memory NOT freed");
			return -1;
		}
	}
	
	for (i = 0; i < dataLog.numShortVars; i++){
		shortData[i] = malloc (sizeof(uint16_t) * dataLog.logArraySize);
		if (shortData[i] == NULL){
			fprintf(stderr, "\nFATAL: Couldn't allocate space for short logging arrays! Memory NOT freed");
			return -1;
		}
	}	
	return 0;
}


//Frees the memory used by logging arrays
void close_datalogger ()
{
	//Write logging data to in MATLAB format to RAMFILE_NAME
	//writeMATLAB();
	
	int i;
	
	for (i = 0; i < dataLog.numDoubleVars; i++)
		free (doubleData[i]);

	for (i = 0; i < dataLog.numFloatVars; i++)
		free (floatData[i]);
	
//	for (i = 0; i < dataLog.numXrayVars; i++)
//		free (xrayData[i]);
	
	for (i = 0; i < dataLog.numIntVars; i++)
		free(intData[i]);
	
	for (i = 0; i < dataLog.numShortVars; i++)
		free(shortData[i]);
	
	return;
}


//Writes values to arrays
//Assumes the arrays have been initialized with initLogging, returns immediately if no space is left in the arrays.
void datalogger (struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr, uint16_t cpuLoad)
{
	if (currentDataPoint >= dataLog.logArraySize)
		return;
	
	int i;
	
	for (i = 0; i < dataLog.numDoubleVars; i++)
		(doubleData[i])[0] = (double)*(dataLog.saveAsDoublePointers[i]);

	for (i = 0; i < dataLog.numFloatVars; i++)
		(floatData[i])[0] = (float)*(dataLog.saveAsFloatPointers[i]);
	
	for (i = 0; i < dataLog.numIntVars; i++)
		(intData[i])[0] = (int32_t)*(dataLog.saveAsIntPointers[i]);
	
	for (i = 0; i < dataLog.numShortVars; i++)
		(shortData[i])[0] = (uint16_t)*(dataLog.saveAsShortPointers[i]);
	
	currentDataPoint++;
	return;
}

int xraylogger (struct sensordata sensorData_ptr)
{
	
	int full = 0;
	if (currentXrayPoint >= dataLog.logArraySize)
	return 1;
	
	p1[currentXrayPoint] = sensorData_ptr.xrayData_ptr->time;
	p3[currentXrayPoint] = sensorData_ptr.xrayData_ptr->peak;
	
	currentXrayPoint++;
	
	if(currentXrayPoint >= 20)		//this should switch between buffers
	{   
		
		full = 1;
		temp1=p2;
		p2=p1;
		p1=temp1;
		
		temp2=p4;		//Append: I2C
		p4=p3;			//Append: I2C
		p3=temp2;		//Append: I2C
		
		currentXrayPoint=0;
		printf("full");
	}
	
	return full;
}
	//sensorData_ptr.imuData_ptr->time,
	//%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,Done\n",  
	//
int dataprinter (struct sensordata sensorData_ptr, int xray_dump)
{
		
	// Change decimal values to desired accuracy
	printf("Data:%.2f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.1f,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,Done\n",	
			sensorData_ptr.imuData_ptr->time, 																
			sensorData_ptr.imuData_ptr->p,sensorData_ptr.imuData_ptr->q,sensorData_ptr.imuData_ptr->r,      
			sensorData_ptr.imuData_ptr->ax,sensorData_ptr.imuData_ptr->ay,sensorData_ptr.imuData_ptr->az,   
			sensorData_ptr.imuData_ptr->hx,sensorData_ptr.imuData_ptr->hy,sensorData_ptr.imuData_ptr->hz,
			sensorData_ptr.imuData_ptr->T,																		
			sensorData_ptr.gpsData_ptr->GPS_week,(int)((sensorData_ptr.gpsData_ptr->GPS_TOW)/1000),			
			sensorData_ptr.gpsData_ptr->Xe,sensorData_ptr.gpsData_ptr->Ye,sensorData_ptr.gpsData_ptr->Ze,   
			sensorData_ptr.gpsData_ptr->Px,sensorData_ptr.gpsData_ptr->Py,sensorData_ptr.gpsData_ptr->Pz,   
			sensorData_ptr.gpsData_ptr->Ue,sensorData_ptr.gpsData_ptr->Ve,sensorData_ptr.gpsData_ptr->We,   
			sensorData_ptr.gpsData_ptr->Pu,sensorData_ptr.gpsData_ptr->Pv,sensorData_ptr.gpsData_ptr->Pw
			);
	int i;
	if(xray_dump){
		
		printf("Xray:");
		counter1 = currentXrayPoint;
		counter2 = currentXrayPoint;
		currentXrayPoint = 0;
		
		for (i = 0; i < counter1; i++)
		{
			printf(",");
			printf("%.6f", p2[i]);
		}
		
		printf("Peak:");
	
		for (i = 0; i < counter2; i++)
		{
			printf(",");
			printf("%.4f", p4[i]);		//Append: I2C
		}
		
		printf("Done\n");
		xray_dump = 0;
	}
	return xray_dump;
}