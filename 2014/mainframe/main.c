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

#include "sensors/imu/hasp_IMU.h"
#include "sensors/gps/gps_novatel.h"
#include "sensors/peak_adc/peak_ADC.h"
#include "globaldefs.h"

// Global data structures
struct sensordata sensorData;
struct imu imuData;
struct gps gpsData;

// Timing stuff, only for testing
int t, t_0, log_freq;

state checkState(state); // Define the main state machine

void main() {

	//Populate sensorData structure with locations of gpsData and imuData structures
       sensorData.gpsData_ptr = &gpsData;
       sensorData.imuData_ptr = &imuData;
       fprintf(stderr,"after the pointer assignments\n");

       (sensorData.gpsData_ptr)->buffer = malloc(285*sizeof(char));
       fprintf(stderr,"after the GPS buffer allocation\n");

       init_GPS(&gpsData);
       fprintf(stderr,"after calling init_GPS()\n");

       init_IMU(&imuData);
       fprintf(stderr,"after calling init_IMU()\n");

       state STATE = IDLE;
       fprintf(stderr,"after initializing STATE as IDLE\n");

       //Time reference in seconds
       t_0 = (int)time(NULL);
       fprintf(stderr,"time reference has been set\n");

       fprintf(stderr,"right before the while(1)\n");
       while(1)
       {
               t = (int)time(NULL) - t_0;
               log_freq = t/100;

               fprintf(stderr,"t=%d,log_freq=%d\n",t,log_freq);

               //checkFlags();
               STATE = checkState(STATE);
       }


	close_IMU(&imuData);
	return 0;
	
} // End main function

state checkState(state SMSTATE)
{
       switch(SMSTATE)
       {
               case IDLE:
			int intFlag = system(INTERRUPT_CHECK);
			switch (intFlag)
			{
				case 1:
					read_eventA();
					printf("EventA\n");
					break;
				case 2:	
					read_eventB();
					printf("EventB\n");
					break;
				case 3:
					read_eventAB();
					printf("EventAB\n");
					break;
				default:
					break;
			}
                       /*if((g_time() - last_IMU_rd_time >= IMU_READ_PERIOD)
					STATE = RD_IMU;
				else if((g_time() - last_log_time) >= LOG_PERIOD)
					STATE = LOG_DATA;
				else if((g_time() - last_GPS_rd_time) >= GPS_READ_PERIOD)
					STATE = RD_GPS;
				else if((g_time - last_downlink_time) >= DOWNLINK_PERIOD)
					STATE = DOWNLINK;
				else
					STATE = IDLE;
				break; */

			// Debugging:
                       fprintf(stderr,"state = IDLE\n\n\n");
                       SMSTATE = RD_IMU;
                       break;

               case RD_IMU:
                       //read_IMU() reads IMU and populates the imuData structure
                       fprintf(stderr,"state = RD_IMU\n");
                       read_IMU(&imuData);
                       SMSTATE = RD_GPS;
                       break;

               case RD_GPS:
                       //read_GPS() stores the ASCII output in gpsData structure buffer
                       fprintf(stderr,"state = RD_GPS\n");
                       read_GPS(&gpsData);
                       SMSTATE = LOG_DATA;
                       break;

               case LOG_DATA:
                       //log entire sensorData structure, which contains the locations of
                       fprintf(stderr,"state = LOG_DATA\n");
                       datalogger(&sensorData,log_freq);
                       SMSTATE = IDLE;
                       break;

               case DOWNLINK:
                       //print stuff out to the telemetry port
                       fprintf(stderr,"state = DOWNLINK\n");
                       SMSTATE = IDLE;
                       break;

               default:
                       SMSTATE = IDLE;
                       break;
       }

return SMSTATE;
}
