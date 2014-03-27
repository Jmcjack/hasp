/*! \file thor_datalog.h
 *	\brief Thor data logging setup
 *
 *	\details Defines what variables are logged for the Thor aircraft.
 *	\ingroup datalog_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: thor_datalog.h 777 2012-02-23 16:58:44Z murch $
 */
#ifndef HASP_DATALOG_H_
#define HASP_DATALOG_H_
	
// Datalogging setup
#define LOG_ARRAY_SIZE 100 ///< Number of data points in the logging array. 50 Hz * 60 sec/min * 30 minutes = 90000

#define NUM_DOUBLE_VARS 3	///< Number of variables that will be logged as doubles
#define NUM_FLOAT_VARS 20	///< Number of variables that will be logged as floats
#define NUM_XRAY_VARS 1
#define NUM_INT_VARS 2		///< Number of variables that will be logged as ints
#define NUM_SHORT_VARS 5	///< Number of variables that will be logged as shorts

//Names of matlab variables MUST match pointers!!

/// char array of variable names for doubles
char* saveAsDoubleNames[100] = {"time", "p","q"};

/// double pointer array to variables that will be saved as doubles
double* saveAsDoublePointers[100] = {&gpsData.Xe, &gpsData.Ye, &gpsData.Ze};

/// char array of variable names for floats
char* saveAsFloatNames[100] = {
 			"r",
			"ax", "ay", "az", 
			"hx", "hy", "hz", 
			"temp", "week", "secs",
			"x", "y","z", 
			"px", "py", "pz",
			"u",
			"v","w","pu"};
								
/// double pointer array to variables that will be saved as floats
double* saveAsFloatPointers[100] = {
			&imuData.time,
			&imuData.p, &imuData.q, &imuData.r,
			&imuData.ax, &imuData.ay, &imuData.az,
			&imuData.hx, &imuData.hy, &imuData.hz,					
			&gpsData.Xe, &gpsData.Ye, &gpsData.Ze, 
			&gpsData.Ue, &gpsData.Ve, &gpsData.We, 
			&gpsData.GPS_TOW, 
			&etime_daq, &etime_datalog, &etime_telemetry};

char* saveAsXrayNames[100] = {"pulseTime"};

double* saveAsXrayPointers[100] = {
		&xrayData.time};

/// char array of variable names for ints
char* saveAsIntNames[100] = {"pv","pw"};

/// int32_t pointer array to variables that will be saved as ints
int32_t* saveAsIntPointers[100] = {(int32_t *)&imuData.err_type,(int32_t *)&gpsData.err_type};


/// char array of variable names for shorts
char* saveAsShortNames[100] = {"mode", "satVisible", "navValid","cpuLoad","run_num"};

/// uint16_t pointer array to variables that will be saved as shorts
uint16_t* saveAsShortPointers[100] = {&controlData.mode, &gpsData.satVisible,
												  &gpsData.navValid,&cpuLoad,&controlData.run_num};
#endif	

