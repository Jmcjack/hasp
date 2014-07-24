#include <stdio> // <3
#include <stdlib> //hello there

#include "sensors/imu/hasp_IMU.h"
#include "sensors/gps/gps_novatel.h"
#include "globaldefs.h"


void main() {

	// Initializations //
	// Still need to put into main sensor data pointer
	struct imu imuData_ptr;
	struct gps gpsData_ptr;

	// Set up SPI buses
		// ADC - as of 7/20/14, this is done in the kernel.
	init_IMU(&imu_ptr);
		
	// Set up serial buses
	init_GPS(&gpsData);
		// Telemetry serial
		
	// Set up other GPIOs
		// GPS PPS
		// GPS Lock
		// Soft reset pins
		
	// If no errors, downlink an OKGO
	
	// Should we use glib to time of this junk? Or is there a better way for timing?

	while(1)
	{
		switch (STATE)
		{
			case IDLE:
				if((g_time() - last_IMU_rd_time >= IMU_READ_PERIOD)
					STATE = RD_IMU;
				else if((g_time() - last_log_time) >= LOG_PERIOD)
					STATE = LOG_DATA;
				else if((g_time() - last_GPS_rd_time) >= GPS_READ_PERIOD)
					STATE = RD_GPS;
				else if((g_time - last_downlink_time) >= DOWNLINK_PERIOD)
					STATE = DOWNLINK;
				else
					STATE = IDLE;
				break;
				
			case RD_IMU:
				read_imu(&imu_ptr);
				last_IMU_rd_time = g_time();
				break;
			case LOG_DATA:
			//  log_data();
				last_log_time = g_time();
				break;
			case RD_GPS:
				read_GPS(gpsData_ptr);
				last_GPS_rd_time = g_time();
				break;
			case DOWNLINK:
			//  send_downlink();
				last_downlink_time = g_time();
				break;
		} // end STATE switch
		
	// While we are not in the main switch statement, check for interrupts
	// Peak data is handled separately
	system(CHECK_INTERRUPTS);
		
	} // End main while loop

	// Close everything
	close_IMU(&imu_ptr);
	
	return;
	
} // End main function
