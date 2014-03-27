/*! \file scheduling.h
 *	\brief Scheduling functions header
 *
 *	\details This file defines the number of alarms and threads, the time offset of each alarm, and the
 *	execution rate.
 *	\ingroup sched_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: scheduling.h 776 2012-02-21 20:57:48Z murch $
 */

#ifndef SCHEDULING_H_
#define SCHEDULING_H_

#define NUM_ALARMS 11	///< Number of alarms

// Update rates
#define BASE_HZ 300		///< Base frequency of the main thread
#define DATALOGGING_HZ 30	///<Frequency of Datalogging
#define TELEMETRY_HZ 2	///< Frequency of the telemetry function
#define THREAD1_HZ 1	///< Frequency of thread1

// Time offset of each alarm relative to the start of the BASE_HZ frame, units of sec
#define DAQ_OFFSET 			0.000	///< [sec], time offset of daq() relative to the start of the BASE_HZ frame
#define NAV_OFFSET 			0.000	///< [sec], time offset of nav() relative to the start of the BASE_HZ frame
#define GUIDANCE_OFFSET 	0.000	///< [sec], time offset of guidance() relative to the start of the BASE_HZ frame
#define CONTROL_OFFSET 		0.000	///< [sec], time offset of control() relative to the start of the BASE_HZ frame
#define ACTUATORS_OFFSET 	0.000	///< [sec], time offset of actuators() relative to the start of the BASE_HZ frame
#define DATALOG_OFFSET 		0.000	///< [sec], time offset of datalogger() relative to the start of the BASE_HZ frame
#define TELEMETRY_OFFSET 	0.000	///< [sec], time offset of telemetry() relative to the start of the BASE_HZ frame

/*
#define DAQ_OFFSET 			0.0000	///< [sec], time offset of daq() relative to the start of the BASE_HZ frame
#define NAV_OFFSET 			0.0065	///< [sec], time offset of nav() relative to the start of the BASE_HZ frame
#define GUIDANCE_OFFSET 	0.0090	///< [sec], time offset of guidance() relative to the start of the BASE_HZ frame
#define CONTROL_OFFSET 		0.0100	///< [sec], time offset of control() relative to the start of the BASE_HZ frame
#define ACTUATORS_OFFSET 	0.0115	///< [sec], time offset of actuators() relative to the start of the BASE_HZ frame
#define DATALOG_OFFSET 		0.0180	///< [sec], time offset of datalogger() relative to the start of the BASE_HZ frame
#define TELEMETRY_OFFSET 	0.0190	///< [sec], time offset of telemetry() relative to the start of the BASE_HZ frame
#define THREAD1_OFFSET	 	0.0130	///< [sec], time offset of thread1() relative to the start of the BASE_HZ frame
*/

#define THREAD1_OFFSET	 	0.000	///< [sec], time offset of thread1() relative to the start of the BASE_HZ frame

// Conditional variables, defined in main.c
extern pthread_cond_t  trigger_daq, trigger_nav, trigger_guidance, \
				trigger_control, \
				trigger_actuators,  trigger_datalogger, trigger_telemetry,\
				trigger_thread1;

/*! \addtogroup sched_fcns
 * @{*/
/* function prototypes */
// Alarm handler functions
void alarm_handlerfn_daq ( cyg_handle_t alarm, cyg_addrword_t data );		///< daq() alarm handler function
void alarm_handlerfn_nav ( cyg_handle_t alarm, cyg_addrword_t data );		///< nav() alarm handler function
void alarm_handlerfn_guidance ( cyg_handle_t alarm, cyg_addrword_t data );	///< guidance() alarm handler function
void alarm_handlerfn_control ( cyg_handle_t alarm, cyg_addrword_t data );	///< control() alarm handler function
void alarm_handlerfn_actuators ( cyg_handle_t alarm, cyg_addrword_t data );	///< actuators() alarm handler function
void alarm_handlerfn_datalogger ( cyg_handle_t alarm, cyg_addrword_t data );///< datalogger() alarm handler function
void alarm_handlerfn_telemetry ( cyg_handle_t alarm, cyg_addrword_t data );	///< telemetry() alarm handler function
void alarm_handlerfn_thread1 ( cyg_handle_t alarm, cyg_addrword_t data );	///< thread1() alarm handler function

// scheduling functions
void close_scheduler(void);	///< Delete alarms and trigger variables
void init_scheduler(void);	///< Create and initialize alarms and initialize trigger variables

/*! @}*/
#endif
