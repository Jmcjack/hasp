/*! \file scheduling.c
 *	\brief Scheduling functions.
 *
 *	\details This file creates and starts threads, alarms, and conditional variables used to control
 *	thread and function execution.
 *	\ingroup sched_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: scheduling.c 770 2012-02-14 15:32:39Z murch $
 */
#include <pthread.h>
#include <cyg/kernel/kapi.h>

#include "../globaldefs.h"
#include "../extern_vars.h"
#include "scheduling.h"

static cyg_handle_t rtclock, rtc_counter;
static cyg_handle_t alarm_hdl[NUM_ALARMS];
static cyg_alarm alarm_obj[NUM_ALARMS];
static cyg_resolution_t rtc_resolution;

void init_scheduler(void)
{
	int rtc_ticks_per_sec;

	/* get handle to the system realtime clock */
	rtclock = cyg_real_time_clock ();
	rtc_resolution = cyg_clock_get_resolution ( rtclock );
	rtc_ticks_per_sec = rtc_resolution.divisor;
	cyg_clock_to_counter ( rtclock, &rtc_counter );
	
	// Real time clock frequency must be changed in eCos configuration, as there is another
	// macro that depends on its value.
	//fprintf ( stderr, "cyg_realtime_clock() resolution: dividend=%d, \t divisor=%d\n", rtc_resolution.dividend, rtc_resolution.divisor );

	// Create alarms
	cyg_alarm_create ( rtc_counter, alarm_handlerfn_daq,0,&alarm_hdl[0],&alarm_obj[0] );
	cyg_alarm_create ( rtc_counter,	alarm_handlerfn_nav,0,&alarm_hdl[1],&alarm_obj[1] );
	cyg_alarm_create ( rtc_counter,	alarm_handlerfn_guidance,0,&alarm_hdl[2],&alarm_obj[2] );
	cyg_alarm_create ( rtc_counter,	alarm_handlerfn_control,0,&alarm_hdl[4],&alarm_obj[4] );
	cyg_alarm_create ( rtc_counter, alarm_handlerfn_actuators,0,&alarm_hdl[7],&alarm_obj[7] );
	cyg_alarm_create ( rtc_counter, alarm_handlerfn_datalogger,0,&alarm_hdl[8],&alarm_obj[8] );
	cyg_alarm_create ( rtc_counter, alarm_handlerfn_telemetry,0,&alarm_hdl[9],&alarm_obj[9] );
	cyg_alarm_create ( rtc_counter, alarm_handlerfn_thread1,0,&alarm_hdl[10],&alarm_obj[10] );

	// Initialize conditional variables
	pthread_cond_init (&trigger_daq, NULL);
	pthread_cond_init (&trigger_nav, NULL);
	pthread_cond_init (&trigger_guidance, NULL);
	pthread_cond_init (&trigger_control, NULL);
	pthread_cond_init (&trigger_actuators, NULL);	
	pthread_cond_init (&trigger_datalogger, NULL);
	pthread_cond_init (&trigger_telemetry, NULL);
	pthread_cond_init (&trigger_thread1, NULL);

	// Start alarms
	// DAQ, executes at t0 + 0.0000
	cyg_alarm_initialize ( alarm_hdl[0], DAQ_OFFSET*rtc_ticks_per_sec, rtc_ticks_per_sec/BASE_HZ );

	// NAV, executes at t0 + 0.0050
	cyg_alarm_initialize ( alarm_hdl[1], NAV_OFFSET*rtc_ticks_per_sec, rtc_ticks_per_sec/BASE_HZ );

	// GUIDANCE, executes at t0 + 0.0060
	cyg_alarm_initialize ( alarm_hdl[2], GUIDANCE_OFFSET*rtc_ticks_per_sec, rtc_ticks_per_sec/BASE_HZ );
	
	// CONTROL, executes at t0 + 0.0070
	cyg_alarm_initialize ( alarm_hdl[4], CONTROL_OFFSET*rtc_ticks_per_sec, rtc_ticks_per_sec/BASE_HZ );

	// ACTUATORS, executes at t0 + 0.0085
	cyg_alarm_initialize ( alarm_hdl[7], ACTUATORS_OFFSET*rtc_ticks_per_sec, rtc_ticks_per_sec/BASE_HZ);

	// DATALOGGER, executes at t0 + 0.0180
	cyg_alarm_initialize ( alarm_hdl[8], DATALOG_OFFSET*rtc_ticks_per_sec, rtc_ticks_per_sec/BASE_HZ);
	
	// TELEMETRY, executes at t0 + 0.0190
	cyg_alarm_initialize ( alarm_hdl[9], TELEMETRY_OFFSET*rtc_ticks_per_sec, rtc_ticks_per_sec/TELEMETRY_HZ );

	// Thread1, executes at t0 + 0.0130
	cyg_alarm_initialize ( alarm_hdl[10], THREAD1_OFFSET*rtc_ticks_per_sec, rtc_ticks_per_sec/THREAD1_HZ );

}

/* destroy alarm objects and pthread_cond objects */
void close_scheduler(void)
{
	int i;

	for (i=0; i<NUM_ALARMS; i++)
		cyg_alarm_delete (alarm_hdl[i]);

	pthread_cond_destroy (&trigger_daq);
	pthread_cond_destroy (&trigger_nav);
	pthread_cond_destroy (&trigger_guidance);
	pthread_cond_destroy (&trigger_control);
	pthread_cond_destroy (&trigger_actuators);
	pthread_cond_destroy (&trigger_datalogger);
	pthread_cond_destroy (&trigger_telemetry);	
	pthread_cond_destroy (&trigger_thread1);
}


/* alarm handler functions */
void alarm_handlerfn_daq ( cyg_handle_t alarm, cyg_addrword_t data ){pthread_cond_signal (&trigger_daq);}
void alarm_handlerfn_nav ( cyg_handle_t alarm, cyg_addrword_t data ){pthread_cond_signal (&trigger_nav);}
void alarm_handlerfn_guidance ( cyg_handle_t alarm, cyg_addrword_t data ){pthread_cond_signal (&trigger_guidance);}
void alarm_handlerfn_control ( cyg_handle_t alarm, cyg_addrword_t data ){pthread_cond_signal (&trigger_control);}
void alarm_handlerfn_actuators ( cyg_handle_t alarm, cyg_addrword_t data ){pthread_cond_signal (&trigger_actuators);}
void alarm_handlerfn_telemetry ( cyg_handle_t alarm, cyg_addrword_t data ){pthread_cond_signal (&trigger_telemetry);}
void alarm_handlerfn_datalogger ( cyg_handle_t alarm, cyg_addrword_t data ){pthread_cond_signal (&trigger_datalogger);}
void alarm_handlerfn_thread1 ( cyg_handle_t alarm, cyg_addrword_t data ){pthread_cond_signal (&trigger_thread1);}

