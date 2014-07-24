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

    char buf[1] = {0xD1}; 	// equivalent to 0x11010001 
    if(write(file, buf, 1) !=1){
    	//cout << "Failed to Reset Address in readFullSensorState() " << endl;
    }

	/* After receiving and decoding the slave address the DS1307 outputs an 
	acknowledge on SDA. The DS1307 then begins to transmit data starting with the register address 
	pointed to by the register pointer. If the register pointer is not written to before the initiation of a read 
	mode the first address that is read is the last one stored in the register pointer. The register pointer 
	automatically increments after each byte are read. */
	
	// 7 - Year, Month, Date, Day, Hour, Minute, Second
	// 6 - Month, Date, Day, Hour, Minute, Second ...
	
    int numberBytes = 6;	// change to number of bytes we want to read
    int dataBuffer[numberBytes];
	int bytesRead = read(file, dataBuffer, numberBytes);
    if (bytesRead == -1){
    	//cout << "Failure to read Byte Stream in readFullSensorState()" << endl;
    }
	// Store the timestamp as in integer only (ignore the 'day of the week')
	int i;
	int timestamp = 0;
	timestamp = dataBuffer[0];				 // second
	timestamp += databuffer[1]*100; 		 // minute
	timestamp += databuffer[2]*10000; 		 // hour, 24 hour clock
	timestamp += databuffer[1]*1000000; 	 // date of day
	timestamp += databuffer[1]*100000000;	 // month 
	// Omitting year
	
    close(file);

    return timestamp;
}