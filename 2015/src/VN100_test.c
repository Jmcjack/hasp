// Test file for VN100 with tomcat

#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "serial/serial.h"
#include "VN100.h"

#define PORT "/dev/ttyS1"

int init_vn100();
int read_vn100(int);

void main() {
	
	int fd, res;
	fd = init_vn100();
	if (fd < 0)
	{
//		printf("Failed to initialize VN100.  Returning.\n");
		return;
	}
//	printf("Open success\n");
	while (res != -1)
	{
		fprintf(stderr,"Iteration begin");
		res = read_vn100(fd);
		//printf("Iteration done\n");
	}
//	printf("Exception - exiting program.\n");
	
	return;
}

int init_vn100() {
	
	int fd = open (PORT, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
	        //error_message ("error %d opening %s: %s", errno, portname, strerror (errno));
//	        printf("Failed to open port!\n");
		return -1;
	}
	set_interface_attribs(fd, B115200, 0);
	set_blocking(fd, 0);
	write(fd, vn100_async_none, sizeof(vn100_async_none)); // Turn off the async data
	
//	printf("Write successful!\n");
	
//	sleep(2);
//	tcflush(fd,TCIOFLUSH);

	return fd;
}

int read_vn100(int fd) {
	
	int bytesToRead;
	char *dataBuffer;

	

	write(fd, &readCalibratedData[0], sizeof(readCalibratedData));
//	printf("Write 2 success.\n");
	usleep(1000000); // Arbitrary right now
	
	ioctl(fd, FIONREAD, &bytesToRead);
	
	//bytesToRead = 64;
//	printf("Bytes available: %d\n",bytesToRead);
	if (bytesToRead == 0)
		return 1;
	
	read(fd, &dataBuffer[0], bytesToRead);
	
//	printf("Read success.\n");

	int i;
	for (i=0; i<bytesToRead-2;i++)
	{
		fprintf(stderr,"%c",dataBuffer[i]);
//		fprintf(stdout,"%c",dataBuffer[i]);
	}
	fprintf(stderr,"!!!");
	fprintf(stderr,"\n");
//	dataBuffer[0] = '\0';

	return 1;
}
