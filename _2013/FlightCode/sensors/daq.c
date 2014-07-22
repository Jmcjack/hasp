/*! \file daq.c
 *	\brief Data acquisition source code
 *
 *	\details This file implements the init_daq() and get_daq() functions for the UAV.
 *	\ingroup daq_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: daq.c 775 2012-02-21 19:54:46Z murch $
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
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>
#include "../utils/I2C/hasp_i2c_mpc5xxx.h"		// Append: I2C driver for MPC5200, if we use this library

#include "../globaldefs.h"
#include "../utils/serial_mpc5200.h"
#include "../utils/misc.h"
#include "../utils/scheduling.h"
#include "../extern_vars.h"
#include "GPS/gps_interface.h"
#include "IMU/imu_interface.h"
#include "ADC/adc_interface.h"
#include "GPIO/gpio_interface.h"
#include "../navigation/nav_interface.h"
#include "daq_interface.h"
#include AIRCRAFT_UP1DIR


void init_daq(struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr)
{
	// Assign GPS serial port info.
	sensorData_ptr->gpsData_ptr->baudRate = GPS_BAUDRATE;
	sensorData_ptr->gpsData_ptr->portName = GPS_PORT;
	
	/* Initialize sensors */
	init_gps(sensorData_ptr->gpsData_ptr);		/* GPS */
	init_imu();		/* IMU */		
	init_gpio();	
		
	// initialize as no data, no lock
	sensorData_ptr->gpsData_ptr->err_type = got_invalid;
	sensorData_ptr->gpsData_ptr->navValid = 1;
	navData_ptr->err_type = got_invalid;

}
void get_xray(struct sensordata *sensorData_ptr){
	struct xray *xrayData_ptr = sensorData_ptr->xrayData_ptr;
	read_imu_time(xrayData_ptr);
}

void get_daq(struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr){		

	// local pointers to keep things tidy
	struct imu *imuData_ptr = sensorData_ptr->imuData_ptr;
	struct gps *gpsData_ptr = sensorData_ptr->gpsData_ptr;
	

	// IMU Sensor
	read_imu(imuData_ptr);		

	//**** GPS Sensor(s) ****
	
	if(-1 == read_gps(gpsData_ptr)){
		if(++gpsData_ptr->read_calls > BASE_HZ)
			gpsData_ptr->err_type = got_invalid;
	}else {gpsData_ptr->read_calls=0;}
	//**** End GPS ****
	
	// Read GPIOs; data dump, control mode
	read_gpio(controlData_ptr);
	
}

