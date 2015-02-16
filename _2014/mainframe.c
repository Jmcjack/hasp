#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>

#include "simple_gpio.h"
#include "globaldefs.h"
#include "datalogger.h"
#include "gps_novatel.h"
#include "updateEventCounter.h"
#include "spi.h"
#include "HASP_SPI_devices.h"

//Data structures
struct sensordata sensorData;
struct imu imuData;
struct gps gpsData;
struct xray xrayData;

//Timestamps stored as longs
unsigned long t, t_0, log_period;
unsigned long imuStamp,gpsStamp,telStamp,eventStamp;

//State Machine State
typedef enum SM_State {
	IDLE,RD_PEAK,RD_IMU,RD_GPS,DOWNLINK,EVENT_UPDATE,LOG_DATA
} state;

state checkState(state SMSTATE)
{
	long time = get_timestamp_ms();
	switch(SMSTATE)
	{			
		case IDLE:
			if((time-imuStamp) >= 30) {
				SMSTATE = RD_IMU;
			}
			else if((time-gpsStamp) >= 1000) {
				SMSTATE = RD_GPS;
			}
                        else if((time-telStamp) >= 5000) {
                                SMSTATE = DOWNLINK;}
			else if((time-eventStamp) >= 5000) {
				SMSTATE = EVENT_UPDATE;}
			else {
				SMSTATE = IDLE;}
			break;

		case RD_IMU:
//			fprintf(stderr,"state =	RD_IMU\n");
			if(!read_IMU(&imuData))
			{
				IMUlogger(&imuData,log_period);
			}
			imuStamp = get_timestamp_ms();
			SMSTATE = IDLE;
			break;
			
		case RD_GPS:
//			fprintf(stderr,"state =	\t\tRD_GPS\n");
			if(!read_GPS(&gpsData))
			{
				GPSlogger(&gpsData,log_period);
			}
			gpsStamp = get_timestamp_ms();
			SMSTATE = IDLE;
			break;
		
		case DOWNLINK:
			//fprintf(stderr,"state =\tDOWNLINK\n");
			send_telemetry(&sensorData);
			telStamp = get_timestamp_ms();
			SMSTATE = IDLE;
			break;		
		case EVENT_UPDATE:	
//			fprintf(stderr,"state =	\tEVENT\n");
			updateEventCounter(&xrayData);
			eventStamp = get_timestamp_ms();
			SMSTATE = IDLE;
		default:
//			fprintf(stderr,"state = default\n");
			SMSTATE = IDLE;
			break;			
	}

return SMSTATE;
}

int main()
{
	char init_string[75]  = "Main Loop Initialized!\n";
        int fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY);
	if(fd>0)
	{
		write(fd, &init_string[0], 75);
		close(fd);
	}
//Time reference in seconds
        t_0 = get_timestamp_ms();
        log_period = 0;
//	fprintf(stderr,"time reference has been set\n");

	//Populate sensorData structure with locations of gpsData and imuData structures
	sensorData.gpsData_ptr = &gpsData;
	sensorData.imuData_ptr = &imuData;
	sensorData.xrayData_ptr = &xrayData;
//        fprintf(stderr,"after the pointer assignments\n");	

        init_GPS(&gpsData);
        //fprintf(stderr,"after calling init_GPS()\n");

        init_IMU(&imuData);
        //fprintf(stderr,"after calling init_IMU()\n");

	init_telemetry();
	//fprintf(stderr,"after calling init_telemetry()\n");

        state STATE = IDLE;
        //fprintf(stderr,"after initializing STATE as IDLE\n");

	sensorData.xrayData_ptr->countsA = (int)malloc(sizeof(int));
       	sensorData.xrayData_ptr->countsB = (int)malloc(sizeof(int));
	sensorData.xrayData_ptr->countsAB = (int)malloc(sizeof(int));
	
       	sensorData.xrayData_ptr->countsA = 0;
       	sensorData.xrayData_ptr->countsB = 0;
       	sensorData.xrayData_ptr->countsAB = 0;
	
	usleep(500000);
	
  //      fprintf(stderr,"right before the while(1)\n");
        while(1)
        {
                t = get_timestamp_ms() - t_0;
                log_period = t/360000000000;
                STATE = checkState(STATE);
		//eventCopier();
        }


close_IMU(&imuData);
return 0;
}

