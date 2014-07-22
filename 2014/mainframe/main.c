#include <stdio> // <3
#include <stdlib> //hello there
#include <glib.h>

#include "sensors/hasp_IMU.h"
#include "globaldefs.h"

gboolean g_read_IMU(gpointer);

void main() {

	// Initializations //
	
	// setup inital data structures
	imu imu_ptr;
	

	// Set up SPI buses
		// ADC - as of 7/20/14, this is done in the kernel.
	init_IMU(&imu_ptr);
		
	// Set up serial buses
		// GPS serial
		// Telemetry serial
		
	// Set up other GPIOs
		// GPS PPS
		// GPS Lock
		// Soft reset pins
		
	// If no errors, downlink an OKGO
	
	// Should we use glib to time of this junk?
	while(1)
	{
		GMainLoop* mainLoop = g_main_loop_new(NULL, FALSE);
		guint IMU_ID = g_timeout_add (1000, g_read_IMU, &imu_ptr);
		g_main_loop_run(*mainLoop);
		
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
