/*! \file telemetry.c
 *	\brief Send telemetry data through serial port
 *
 *	\details
 *	\ingroup telemetry_fcns
 *
 * \author University of Minnesota
 * \author Aerospace Engineering and Mechanics
 * \copyright Copyright 2011 Regents of the University of Minnesota. All rights reserved.
 *
 * $Id: telemetry.c 761 2012-01-19 17:23:49Z murch $
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <sched.h>
#include <cyg/posix/pthread.h>
#include <cyg/kernel/kapi.h>
#include <cyg/cpuload/cpuload.h>
#include <cyg/io/io.h>
#include <cyg/io/serialio.h>

#include "../globaldefs.h"
#include "../utils/serial_mpc5200.h"
#include "../utils/misc.h"
#include "../extern_vars.h"
#include "telemetry_interface.h"
#include AIRCRAFT_UP1DIR

#define TELE_PACKET_SIZE  44
extern char statusMsg[103];	

/* send_telemetry packet structure = [ <UUT> time, <empty>, <empty>, p, q, r, alt, IAS, psi, theta, phi, rud, ele, thr, ail, cpuload, lon, lat, statusFlags, <empty> <16bit_CKSUM> ] */

static int port;

void init_telemetry(){
	// Open serial port for send_telemetry. Set in /aircraft/xxx_config.h
	port = open_serial(TELEMETRY_PORT, TELEMETRY_BAUDRATE);	
}

void send_telemetry(struct sensordata *sensorData_ptr, struct nav *navData_ptr, struct control *controlData_ptr, uint16_t cpuLoad)
{
	int bytes=0;
	static byte sendpacket[TELE_PACKET_SIZE];
	char string_to_write[] = "\0\0\0\0\0\0\0\0";
	sprintf(string_to_write,"u,");
	memcpy(&sendpacket[0],string_to_write,2);
	sprintf(string_to_write,"%d,", (int)(sensorData_ptr->imuData_ptr->time));
	memcpy(&sendpacket[2],string_to_write,7);
	sprintf(string_to_write,"%d,", (int)(sensorData_ptr->gpsData_ptr->Xe));
	memcpy(&sendpacket[9],string_to_write,9);
	sprintf(string_to_write,"%d,", (int)(sensorData_ptr->gpsData_ptr->Ye));
	memcpy(&sendpacket[18],string_to_write,9);
	sprintf(string_to_write,"%d,", (int)(sensorData_ptr->gpsData_ptr->Ze));
	memcpy(&sendpacket[27],string_to_write,9);
	sprintf(string_to_write,"%.1f,", sensorData_ptr->imuData_ptr->T);
	memcpy(&sendpacket[36],string_to_write,5);
	sprintf(string_to_write,"5\n\r");
	memcpy(&sendpacket[41],string_to_write,3);

//	// send send_telemetry data packet to serial port
	while(bytes != TELE_PACKET_SIZE) bytes += write(port, &sendpacket[bytes], TELE_PACKET_SIZE-bytes); bytes=0;
}
