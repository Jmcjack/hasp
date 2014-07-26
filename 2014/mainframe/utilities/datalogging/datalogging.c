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

#include "../../globaldefs.h"
#include "../../sensors/gps/gps_novatel.h"
#include "../spi/spi.h"
#include "../../hasp_imu.h"
#include "datalogger.h"


int datalogger(struct sensordata *sensorData_ptr, uint8_t log_number)
{

       //The argument log_number is used to determine the filename that is to
       // be logged to. In the main loop of flight code a variable should be
       //  incremented periodically so that every so often a new IMUlog**.txt
       //  or GPSlog**.txt can be created.
       char gps_filename[] = "/mnt/microSD/flightData/GPSlog00.txt";
       gps_filename[30] = log_number/10 + '0';
       gps_filename[31] = log_number%10 + '0';
       char imu_filename[] = "/mnt/microSD/flightData/IMUlog00.txt";
       imu_filename[30] = log_number/10 + '0';
       imu_filename[31] = log_number%10 + '0';


       //Opening the files as "a" means that whatever we want to put
       //  into them is appended to the existing file. If it doesn't
       //  exist, fopen() will create a new one.
       fprintf(stderr,"opening %s for the GPS data and %s for the IMU data\n",gps_filename,imu_filename);
       FILE *gpsFile = fopen(gps_filename,"a");
       FILE *imuFile = fopen(imu_filename,"a");


       //If either of the files were not opened correctly fopen() returns
       //  a negative integer; in this unlikely case we exit datalogger().
       if(!imuFile){
                    fprintf(stderr,"imuFile=%d\n",imuFile);
                       fprintf(stderr,"error opening IMU file. exiting now\n");
                       return -1;
       }else{
                       fprintf(stderr,"imuFile=%d\n",imuFile);}
       if(!gpsFile){
                       fprintf(stderr,"imuFile=%d\n",imuFile);
                       fprintf(stderr,"error opening GPS file. exiting now\n");
                       return -1;
       }else{
                       fprintf(stderr,"imuFile=%d\n",imuFile);}


       //Output from the GPS receiver is in the form of an ASCII string.
       //All that is needed is to copy the buffer from the gpsData structure
       //  into the data file. The length of the string varies in length but
       //  is roughly 260 bytes (the last of which is an \n), so we took a
       //  conservative estimate to ensure no data loss.
       fwrite(sensorData_ptr->gpsData_ptr->buffer,sizeof(char),285,gpsFile);
       fprintf(stderr,"after GPS data is written\n");

       //Output data from the IMU is stored in 32-bit integers, so the
       //  file containing output will contain the RAW format data;
       //  formatted into ASCII, comma-delimited, one reading per line.
       //  We did this to enable easy parsing by MATLAB, etc once the
       //  data files are ready to be read and parsed. One set per line.
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->supply_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->gyroX_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->gyroY_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->gyroZ_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->accelX_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->accelY_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->accelZ_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->magX_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->magY_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->magZ_raw);
       fprintf(imuFile,"%d,",sensorData_ptr->imuData_ptr->temp_raw);
       fprintf(imuFile,"%d\n",sensorData_ptr->imuData_ptr->ADC_raw);
       fprintf(stderr,"after IMU data is written\n");

       //Close the files after the logging. The next datalogger() call will append
       //  the new set of data to the existing files if log_number doesn't change.
       fclose(imuFile);
       fclose(gpsFile);

       fprintf(stderr,"after imuFile and gpsFile have been closed\n");


return 0;
}
