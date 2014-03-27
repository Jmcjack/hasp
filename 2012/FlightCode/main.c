/*! \file main.c
 *	\brief Main function, thread 0
 *
 *	\details The main function is here, which creates and runs all threads. It also
 *	is thread 0, the highest priority thread, which runs the primary avionics software
 *	modules.
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: main.c 774 2012-02-20 21:36:08Z murch $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cyg/kernel/kapi.h>
#include <cyg/cpuload/cpuload.h>

#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>
#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/dev_spipsc.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/infra/cyg_ass.h>      // assertion support
#include <cyg/infra/diag.h>         // diagnostic output
#include <cyg/infra/diag.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/mpc5xxx.h>
#include <cyg/io/spipsc.h>
#include CYGBLD_HAL_PLATFORM_H
#include <cyg/hal/drv_api.h>

#include <cyg/io/mpc5xxx_gpio.h>
#include "extern_vars.h"
#include "utils/misc.h"

#include "utils/I2C/hasp_i2c_mpc5xxx.h"

#include "globaldefs.h"
#include "utils/misc.h"
#include "utils/scheduling.h"
#include "threads/threads.h"

// Interfaces //
#include "sensors/daq_interface.h"
#include "actuators/actuator_interface.h"
#include "navigation/nav_interface.h"
#include "control/control_interface.h"
#include "datalog/datalog_interface.h"
#include "telemetry/telemetry_interface.h"

#include AIRCRAFT


// dataLog structure. External but used only in datalogger.c
struct datalog dataLog;

static cyg_interrupt int1;
static cyg_handle_t int1_handle;

uint16_t count_ = 0;
int xray_dump = 0;

// sensor data
// struct sensordata sensorData;


// Conditional variables. These are external but only used in scheduling.c
pthread_cond_t  trigger_daq, trigger_nav, trigger_guidance, \
trigger_control, trigger_actuators,  trigger_datalogger, trigger_telemetry;

static cyg_uint32 interrupt_1_isr(cyg_vector_t vector, cyg_addrword_t data)
{
	cyg_drv_interrupt_mask(vector);    
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR);  // Cause DSR to be run
}

/*
 * SPIPSC function.
 * Called by upper layer device driver.
 * high level interrupt handler (DSR)
 */

static void interrupt_1_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
	if(!xray_dump){
		get_xray(&sensorData);
		xray_dump = xraylogger(sensorData);
	}
	
	GPIO_GPW_ClearInterrupt(MPC5XXX_GPW_GPIO_WKUP_7);
	cyg_drv_interrupt_acknowledge(vector);
	cyg_drv_interrupt_unmask(vector);
	
	if(i2c_flag == 0)
		get_peak_data(sensorData.xrayData_ptr); // to begin the I2C transaction
	
	// Note: The peak data is stored one array placement behind the associated X-ray peak.
	
	//printf("dsr done\n\r");
}


/// Main function, primary avionics functions, thread 0, highest priority.
int main(int argc, char **argv) {

	// Data Structures
	struct  imu   imuData;
	struct  xray  xrayData;
	struct  gps   gpsData;
	struct  nav   navData;
	struct  control controlData;

	uint16_t cpuLoad;

	// Timing variables
	double etime_daq, etime_datalog, etime_telemetry;

	// Include datalog definition
	#include DATALOG_CONFIG

	// Populate dataLog members with initial values //
	dataLog.saveAsDoubleNames = &saveAsDoubleNames[0];
	dataLog.saveAsDoublePointers = &saveAsDoublePointers[0];
	dataLog.saveAsFloatNames = &saveAsFloatNames[0];
	dataLog.saveAsFloatPointers = &saveAsFloatPointers[0];
	dataLog.saveAsXrayNames = &saveAsXrayNames[0];
	dataLog.saveAsXrayPointers = &saveAsXrayPointers[0];
	dataLog.saveAsIntNames = &saveAsIntNames[0];
	dataLog.saveAsIntPointers = &saveAsIntPointers[0];
	dataLog.saveAsShortNames = &saveAsShortNames[0];
	dataLog.saveAsShortPointers = &saveAsShortPointers[0];
	dataLog.logArraySize = LOG_ARRAY_SIZE;
	dataLog.numDoubleVars = NUM_DOUBLE_VARS;
	dataLog.numFloatVars = NUM_FLOAT_VARS;
	dataLog.numXrayVars = NUM_XRAY_VARS;
	dataLog.numIntVars = NUM_INT_VARS;
	dataLog.numShortVars = NUM_SHORT_VARS;

	double tic,time,t0=0;
	static int t0_latched = FALSE;
	int loop_counter = 0;
	pthread_mutex_t	mutex;

	uint32_t cpuCalibrationData;//, last100ms, last1s, last10s;
	cyg_cpuload_t cpuload;
	cyg_handle_t loadhandle;

	// Populate sensorData structure with pointers to data structures //
	sensorData.imuData_ptr = &imuData;
	sensorData.gpsData_ptr = &gpsData;
	sensorData.xrayData_ptr = &xrayData;

	// Set main thread to highest priority //
	struct sched_param param;
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);

	// Setup CPU load measurements //
	cyg_cpuload_calibrate(&cpuCalibrationData);
	cyg_cpuload_create(&cpuload, cpuCalibrationData, &loadhandle);

	// Initialize set_actuators (PWM or serial) at zero //
//	init_actuators();
//	set_actuators(&controlData);

	// Initialize mutex variable. Needed for pthread_cond_wait function //
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex);

	// Initialize functions	//
	init_daq(&sensorData, &navData, &controlData);
	init_telemetry();

	while(1){
	
		reset_Time();				// initialize real time clock at zero
		init_scheduler();			// Initialize scheduling
		threads_create();			// start additional threads
		init_datalogger();			// Initialize data logging
	
		//Init Interrupts
		printf("intr init");
		cyg_uint32 uChannel = MPC5XXX_GPW_GPIO_WKUP_7;
		BOOL enable = true;
		BOOL bInput = 1;		
			
		// uChannel initialisation
		GPIO_GPW_EnableGPIO(uChannel, enable);
		GPIO_GPW_SetDirection(uChannel, bInput);
		GPIO_GPW_EnableInterrupt(uChannel, enable, MPC5XXX_GPIO_INTTYPE_RISING);
		GPIO_GPW_ClearInterrupt(uChannel);
		HAL_WRITE_UINT32(MPC5XXX_GPW+MPC5XXX_GPW_ME, 0x01000000);
		cyg_uint32 temp;
		HAL_READ_UINT32(MPC5XXX_ICTL_MIMR, temp);
		HAL_WRITE_UINT32(MPC5XXX_ICTL_MIMR, temp&0xFFFFFEFF);
	
		// wake-up interrupt service routine initialisation
		cyg_vector_t int1_vector = CYGNUM_HAL_INTERRUPT_GPIO_WAKEUP;
		cyg_priority_t int1_priority = 0;
		cyg_bool_t edge = 0;
		cyg_bool_t rise = 1;
		cyg_drv_interrupt_create(int1_vector, int1_priority,0,interrupt_1_isr, interrupt_1_dsr,&int1_handle,&int1);
		cyg_drv_interrupt_attach(int1_handle);
		cyg_drv_interrupt_configure(int1_vector,edge,rise);
		cyg_drv_interrupt_unmask(int1_vector);
			
		hasp_mpc5xxx_i2c_init(); 	// Append: Initialise I2C bus and I2C interrupt routine; device defined in i2c_mpc5xxx.h and .c
			
		cyg_interrupt_enable();
		HAL_ENABLE_INTERRUPTS();

		controlData.mode = 1; 		// initialize to manual mode
		controlData.run_num = 0; 	// reset run counter
		
		//+++++++++++//
		// Main Loop //
		//+++++++++++//
		
		while (controlData.mode != 0) {
			
			// get_peak_data(sensorData.xrayData_ptr); // TEST, do not uncomment
			
			loop_counter++; //.increment loop counter

			//**** DATA ACQUISITION **************************************************
			pthread_cond_wait (&trigger_daq, &mutex);
			tic = get_Time();			
			get_daq(&sensorData, &navData, &controlData);		
			etime_daq = get_Time() - tic - DAQ_OFFSET; // compute execution time
			
			//************************************************************************

			//**** NAVIGATION ********************************************************
//			pthread_cond_wait (&trigger_nav, &mutex);
//			if(navData.err_type == got_invalid){ // check if get_nav filter has been initialized
//				if(gpsData.navValid == 0) // check if GPS is locked
//					init_nav(&sensorData, &navData, &controlData);// Initialize get_nav filter
//			}
//			else
//				get_nav(&sensorData, &navData, &controlData);// Call NAV filter

			//************************************************************************

			if (controlData.mode == 2) { // autopilot mode
				if (t0_latched == FALSE) {
					t0 = get_Time();
					t0_latched = TRUE;
				}
				time = get_Time()-t0; // Time since in auto mode		
			}
			else{
				if (t0_latched == TRUE) {				
					t0_latched = FALSE;
				}
				//reset_control(&controlData); // reset controller states and set get_control surfaces to zero
			} // end if (controlData.mode == 2) 

			// Add trim biases to get_control surface commands
			//add_trim_bias(&controlData);

			//**** DATA LOGGING ******************************************************
			
			pthread_cond_wait (&trigger_datalogger, &mutex);
			datalogger(&sensorData, &navData, &controlData, cpuLoad);
			cyg_drv_interrupt_mask(int1_vector); 
			xray_dump = dataprinter(sensorData, xray_dump);
			resetHigh();							// Reset the peak-detection during datalogging
			cyg_drv_interrupt_unmask(int1_vector); 
			resetLow();								// Reset the peak-detection during datalogging
			etime_datalog = get_Time() - tic - DATALOG_OFFSET; // compute execution time
			
			//************************************************************************

			//**** TELEMETRY *********************************************************
			if(loop_counter >= BASE_HZ/TELEMETRY_HZ){
				loop_counter = 0;
				pthread_cond_wait (&trigger_telemetry, &mutex);
//
				// get current cpu load
//				cyg_cpuload_get (loadhandle, &last100ms, &last1s, &last10s);
//				cpuLoad = (uint16_t)last100ms;
//
				send_telemetry(&sensorData, &navData, &controlData, cpuLoad);
//
//				etime_telemetry = get_Time() - tic - TELEMETRY_OFFSET; // compute execution time
			}
					
			//************************************************************************	
		} //end while (controlData.mode != 0)

		close_scheduler();
		close_datalogger(); // dump data

	} // end while(1)
	/**********************************************************************
	 * close
	 **********************************************************************/
	pthread_mutex_destroy(&mutex);
//	close_actuators();	
	//close_nav();

	return 0;

} // end main

