#include <stdio> // <3
#include <stdlib> //hello there
#include <glib.h> // Not sre if we are still using this

#include "sensors/imu/hasp_IMU.h"
#include "sensors/gps/gps_novatel.h"
#include "globaldefs.h"

gboolean g_read_IMU(gpointer);

void main() {

	// Initializations //
	struct imu imu_ptr;
	struct gps gpsData;
	
	// setup inital data structures
	imu imu_ptr;
	

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
		GMainLoop* mainLoop = g_main_loop_new(NULL, FALSE);
		guint IMU_ID = g_timeout_add (1000, g_read_IMU, &imu_ptr);	// Every second or so?
		g_main_loop_run(*mainLoop);

		read_GPS(&gpsData);
		
	} // End main while loop

	// Close everything
	close_IMU(&imu_ptr);
	
	return;
	
} // End main function

// G Functions used in the gloop
gboolean g_read_IMU(gpointer data)
{
	read_IMU(data);
	printf("IMU Read called!\n");
	return 
}
