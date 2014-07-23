#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include "hasp_rtc.h"

int getTimeStamp(){

   //cout << "Starting BMA180 I2C sensor state read" << endl;
    char namebuf = I2C_PORT;

    int file;
    if ((file = open(namebuf, O_RDWR)) < 0){
            //cout << "Failed to open BMA180 Sensor on " << namebuf << " I2C Bus" << endl;
            return(1);
    }
    if (ioctl(file, I2C_SLAVE, I2CAddress) < 0){
            //cout << "I2C_SLAVE address " << I2CAddress << " failed..." << endl;
            return(2);
    }

    char buf[1] = { 0x00 }; 	// change to command of RTC, if there is one.  It is a good idea to check the first byte.
    if(write(file, buf, 1) !=1){
    	//cout << "Failed to Reset Address in readFullSensorState() " << endl;
    }

    int numberBytes = RTC_I2C_BUFFER;	// change to number of bytes
    int bytesRead = read(file, this->dataBuffer, numberBytes);
    if (bytesRead == -1){
    	//cout << "Failure to read Byte Stream in readFullSensorState()" << endl;
    }
    close(file);

    /*
	if (this->dataBuffer[0]!=0x03){
    	cout << "MAJOR FAILURE: DATA WITH BMA180 HAS LOST SYNC!" << endl;
    }
	*/

    return 0;
}