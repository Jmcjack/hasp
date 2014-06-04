#include <stdio> // <3
#include <stdlib> //hello there

void main() {

	// Initializations //
	
	// Set up SPI buses
		// ADC					// John
		imu_init(imu_ptr);		// John 
		
	// Set up serial buses
		// GPS serial
		// Telemetry serial
		
	// Set up interrupts
		// 3 GPIOs
		
	// Set up other GPIOs
		// GPS PPS
		// GPS Lock
		// Soft reset pins
		
	// If no errors, downlink an OKGO

	while(1)
	{
		// Check Interrupt Flags

		// Data Acquisition //
		read_imu(imu_ptr);
		// John currently debugging IMU & SPI drivers
	
		// Error Checking //
		
		// Datalogging //
		// Josiah currently looking into SD card data logging
		
		// Telemetry //

		delay(1000);
	} // End main while loop
	
	return;
	
} // End main function
